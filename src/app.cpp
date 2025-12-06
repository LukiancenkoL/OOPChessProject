#include "app.hpp"
#include "stockfish.hpp"

// Pieces headers
#include "pawn.hpp"
#include "rook.hpp"
#include "knight.hpp"
#include "bishop.hpp"
#include "queen.hpp"
#include "king.hpp"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <cstdlib>
#include <print>
#include <string>
#include <unordered_map>
#include <algorithm>

char getPieceChar(const std::unique_ptr<Piece>& p) {
	if (!p)
		return ' ';
	char c = '?';
	switch (p->getType()) {
	case PieceType::PAWN:
		c = 'p';
		break;
	case PieceType::ROOK:
		c = 'r';
		break;
	case PieceType::KNIGHT:
		c = 'n';
		break;
	case PieceType::BISHOP:
		c = 'b';
		break;
	case PieceType::QUEEN:
		c = 'q';
		break;
	case PieceType::KING:
		c = 'k';
		break;
	default:
		c = '?';
		break;
	}
	return (p->getColor() == Color::WHITE) ? std::toupper(c) : c;
}

struct AppState {
	Stockfish stockfish;
	bool in_menu = true;
	bool vs_engine = false;
	int difficulty = 5;
	Color player_color = Color::WHITE;
	Color turn = Color::WHITE;
	BoardCoordinates selected_sq = {-1, -1};
	std::vector<BoardCoordinates> valid_moves;
	std::unordered_map<char, SDL_Texture*> textures;
	bool game_over = false;
	std::string status_msg = "Welcome! Choose settings.";
	bool engine_thinking = false;

	std::vector<std::string> move_history;
	bool scroll_to_bottom = false;

	BoardCoordinates en_passant_target = {-1, -1};
};

AppState g_state;

std::string coordsToString(int x, int y) {
	char file = 'a' + x;
	char rank = '8' - y;
	return std::string{file, rank};
}

App::App() {
	if (!SDL_Init(App::init_flags))
		std::exit(EXIT_FAILURE);

	auto main_scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
	this->window = SDL_CreateWindow("Chess",
			App::initial_window_width * static_cast<int32_t>(main_scale),
			App::initial_window_height * static_cast<int32_t>(main_scale), App::window_flags);

	this->renderer = SDL_CreateRenderer(this->window, nullptr);
	SDL_SetRenderVSync(this->renderer, 0);
	SDL_ShowWindow(this->window);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui::GetStyle().ScaleAllSizes(1.5f);

	ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer3_Init(renderer);

	resetBoard();
}

void App::resetBoard() {
	for (auto& p : board)
		p.reset();
	g_state.turn = Color::WHITE;
	g_state.game_over = false;
	g_state.selected_sq = {-1, -1};
	g_state.valid_moves.clear();
	g_state.engine_thinking = false;
	g_state.move_history.clear();
	g_state.en_passant_target = {-1, -1};

	if (!g_state.in_menu)
		g_state.status_msg = "White to move";

	auto place = [&](int x, int y, Piece* p) { board[y * 8 + x].reset(p); };

	for (int i = 0; i < 8; i++) {
		place(i, 1, new Pawn(Color::BLACK, {(int8_t)i, 1}));
		place(i, 6, new Pawn(Color::WHITE, {(int8_t)i, 6}));
	}
	using C = Color;
	place(0, 0, new Rook(C::BLACK, {0, 0}));
	place(7, 0, new Rook(C::BLACK, {7, 0}));
	place(0, 7, new Rook(C::WHITE, {0, 7}));
	place(7, 7, new Rook(C::WHITE, {7, 7}));
	place(1, 0, new Knight(C::BLACK, {1, 0}));
	place(6, 0, new Knight(C::BLACK, {6, 0}));
	place(1, 7, new Knight(C::WHITE, {1, 7}));
	place(6, 7, new Knight(C::WHITE, {6, 7}));
	place(2, 0, new Bishop(C::BLACK, {2, 0}));
	place(5, 0, new Bishop(C::BLACK, {5, 0}));
	place(2, 7, new Bishop(C::WHITE, {2, 7}));
	place(5, 7, new Bishop(C::WHITE, {5, 7}));
	place(3, 0, new Queen(C::BLACK, {3, 0}));
	place(4, 0, new King(C::BLACK, {4, 0}));
	place(3, 7, new Queen(C::WHITE, {3, 7}));
	place(4, 7, new King(C::WHITE, {4, 7}));
}

void App::loadTextures() {
	const char* pieces = "prnbqk";
	const char* colors = "wb";
	for (int c = 0; c < 2; c++) {
		for (int p = 0; p < 6; p++) {
			std::string filename = "piece/";
			filename += colors[c];
			filename += (char)std::toupper(pieces[p]);
			filename += ".svg";
			SDL_Surface* surf = nullptr;
			SDL_IOStream* io = SDL_IOFromFile(filename.c_str(), "rb");
			if (io) {
				surf = IMG_LoadSizedSVG_IO(io, 256, 256);
				SDL_CloseIO(io);
			}
			if (!surf)
				surf = IMG_Load(filename.c_str());
			if (surf) {
				SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
				SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);
				g_state.textures[(c == 0) ? std::toupper(pieces[p]) : pieces[p]] = tex;
				SDL_DestroySurface(surf);
			}
		}
	}
}

// Generate FEN with proper Castling & En Passant
std::string generateFEN(const std::array<std::unique_ptr<Piece>, 64>& board, Color turn) {
	std::string fen = "";
	for (int y = 0; y < 8; y++) {
		int empty = 0;
		for (int x = 0; x < 8; x++) {
			auto& p = board[y * 8 + x];
			if (!p)
				empty++;
			else {
				if (empty > 0) {
					fen += std::to_string(empty);
					empty = 0;
				}
				fen += getPieceChar(p);
			}
		}
		if (empty > 0)
			fen += std::to_string(empty);
		if (y < 7)
			fen += "/";
	}
	fen += (turn == Color::WHITE ? " w " : " b ");

	std::string castling = "";
	auto& wk = board[60];
	if (wk && wk->getType() == PieceType::KING && !wk->hasMoved()) {
		auto& wrk = board[63];
		if (wrk && wrk->getType() == PieceType::ROOK && !wrk->hasMoved())
			castling += "K";
		auto& wrq = board[56];
		if (wrq && wrq->getType() == PieceType::ROOK && !wrq->hasMoved())
			castling += "Q";
	}
	auto& bk = board[4];
	if (bk && bk->getType() == PieceType::KING && !bk->hasMoved()) {
		auto& brk = board[7];
		if (brk && brk->getType() == PieceType::ROOK && !brk->hasMoved())
			castling += "k";
		auto& brq = board[0];
		if (brq && brq->getType() == PieceType::ROOK && !brq->hasMoved())
			castling += "q";
	}
	if (castling.empty())
		castling = "-";
	fen += " " + castling;

	if (g_state.en_passant_target.x != -1) {
		fen += " " + coordsToString(g_state.en_passant_target.x, g_state.en_passant_target.y);
	} else {
		fen += " -";
	}

	fen += " 0 1";
	return fen;
}

bool isSquareAttacked(const std::array<std::unique_ptr<Piece>, 64>& board, BoardCoordinates sq,
		Color defenderColor) {
	for (const auto& p : board) {
		if (p && p->getColor() != defenderColor) {
			auto moves = p->getPossibleMoves(board);
			for (const auto& m : moves) {
				if (m.x == sq.x && m.y == sq.y)
					return true;
			}
		}
	}
	return false;
}

bool isKingInCheck(const std::array<std::unique_ptr<Piece>, 64>& board, Color kingColor) {
	BoardCoordinates kingPos = {-1, -1};
	for (const auto& p : board) {
		if (p && p->getType() == PieceType::KING && p->getColor() == kingColor) {
			kingPos = p->getPosition();
			break;
		}
	}
	if (kingPos.x == -1)
		return false;
	return isSquareAttacked(board, kingPos, kingColor);
}

bool isMoveSafe(
		std::array<std::unique_ptr<Piece>, 64>& board, int from_idx, int to_idx, Color turn) {
	bool is_ep = false;
	int ep_capture_idx = -1;
	if (board[from_idx]->getType() == PieceType::PAWN && to_idx % 8 != from_idx % 8 &&
			board[to_idx] == nullptr) {
		is_ep = true;
		ep_capture_idx = (from_idx / 8) * 8 + (to_idx % 8);
	}

	std::unique_ptr<Piece> captured_piece;
	if (is_ep) {
		captured_piece = std::move(board[ep_capture_idx]);
		board[ep_capture_idx] = nullptr;
	} else {
		captured_piece = std::move(board[to_idx]);
	}

	board[to_idx] = std::move(board[from_idx]);
	BoardCoordinates old_pos = board[to_idx]->getPosition();
	BoardCoordinates new_pos = {(int8_t)(to_idx % 8), (int8_t)(to_idx / 8)};
	board[to_idx]->setPosition(new_pos);

	bool safe = !isKingInCheck(board, turn);

	board[to_idx]->setPosition(old_pos);
	board[from_idx] = std::move(board[to_idx]);

	if (is_ep)
		board[ep_capture_idx] = std::move(captured_piece);
	else
		board[to_idx] = std::move(captured_piece);

	return safe;
}

void checkGameState(std::array<std::unique_ptr<Piece>, 64>& board) {
	bool in_check = isKingInCheck(board, g_state.turn);
	bool has_legal_moves = false;
	for (int i = 0; i < 64; ++i) {
		if (board[i] && board[i]->getColor() == g_state.turn) {
			auto moves = board[i]->getPossibleMoves(board);
			// Add EP moves to check
			if (board[i]->getType() == PieceType::PAWN && g_state.en_passant_target.x != -1) {
				int dir = (g_state.turn == Color::WHITE) ? -1 : 1;
				int py = board[i]->getPosition().y;
				int px = board[i]->getPosition().x;
				if (std::abs(g_state.en_passant_target.x - px) == 1 &&
						g_state.en_passant_target.y == py + dir) {
					moves.push_back(g_state.en_passant_target);
				}
			}

			for (auto& m : moves) {
				int to_idx = m.y * 8 + m.x;
				if (isMoveSafe(board, i, to_idx, g_state.turn)) {
					has_legal_moves = true;
					break;
				}
			}
		}
		if (has_legal_moves)
			break;
	}
	if (!has_legal_moves) {
		g_state.game_over = true;
		g_state.status_msg = in_check ? "Checkmate!" : "Stalemate!";
	} else {
		g_state.status_msg = in_check ?
				"Check!" :
				(g_state.turn == Color::WHITE ? "White to move" : "Black to move");
	}
}

void App::run() {
	loadTextures();
	auto done{false};
	SDL_Event event{};

	while (!done) {
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT)
				done = true;
			if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
					event.window.windowID == SDL_GetWindowID(window))
				done = true;
			if (g_state.in_menu || g_state.game_over)
				continue;

			if (!ImGui::GetIO().WantCaptureMouse && event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				if (g_state.vs_engine && g_state.engine_thinking)
					continue;
				int w, h;
				SDL_GetWindowSize(window, &w, &h);
				float board_dim = std::min(w, h);
				float sq_size = board_dim / 8.0f;
				int bx = static_cast<int>(event.button.x / sq_size);
				int by = static_cast<int>(event.button.y / sq_size);

				if (bx >= 0 && bx < 8 && by >= 0 && by < 8) {
					if (g_state.selected_sq.x == -1) {
						int idx = by * 8 + bx;
						if (board[idx] && board[idx]->getColor() == g_state.turn) {
							if (g_state.vs_engine && g_state.turn != g_state.player_color)
								continue;
							g_state.selected_sq = {(int8_t)bx, (int8_t)by};
							g_state.valid_moves = board[idx]->getPossibleMoves(board);

							if (board[idx]->getType() == PieceType::PAWN &&
									g_state.en_passant_target.x != -1) {
								int dir = (g_state.turn == Color::WHITE) ? -1 : 1;
								if (std::abs(g_state.en_passant_target.x - bx) == 1 &&
										g_state.en_passant_target.y == by + dir) {
									g_state.valid_moves.push_back(g_state.en_passant_target);
								}
							}
						}
					} else {
						if (bx == g_state.selected_sq.x && by == g_state.selected_sq.y) {
							g_state.selected_sq = {-1, -1};
							g_state.valid_moves.clear();
							continue;
						}
						for (auto& m : g_state.valid_moves) {
							if (m.x == bx && m.y == by) {
								int from_idx = g_state.selected_sq.y * 8 + g_state.selected_sq.x;
								int to_idx = by * 8 + bx;

								bool is_castling = false;
								if (board[from_idx]->getType() == PieceType::KING &&
										std::abs(bx - g_state.selected_sq.x) > 1) {
									int step = (bx > g_state.selected_sq.x) ? 1 : -1;
									int mid_x = g_state.selected_sq.x + step;
									BoardCoordinates mid_sq = {(int8_t)mid_x, (int8_t)by};
									if (isKingInCheck(board, g_state.turn) ||
											isSquareAttacked(board, mid_sq, g_state.turn)) {
										std::println("Illegal Castle!");
										break;
									}
									is_castling = true;
								}

								if (isMoveSafe(board, from_idx, to_idx, g_state.turn)) {
									std::string move_str = coordsToString(g_state.selected_sq.x,
																   g_state.selected_sq.y) +
											coordsToString(bx, by);
									g_state.move_history.push_back(move_str);
									g_state.scroll_to_bottom = true;

									BoardCoordinates next_ep_target = {-1, -1};
									if (board[from_idx]->getType() == PieceType::PAWN &&
											std::abs(by - g_state.selected_sq.y) == 2) {
										next_ep_target = {
											(int8_t)bx, (int8_t)((g_state.selected_sq.y + by) / 2)};
									}

									if (board[from_idx]->getType() == PieceType::PAWN &&
											bx == g_state.en_passant_target.x &&
											by == g_state.en_passant_target.y) {
										int capture_idx = g_state.selected_sq.y * 8 + bx;
										board[capture_idx] = nullptr;
									}

									board[to_idx] = std::move(board[from_idx]);
									board[to_idx]->setPosition({(int8_t)bx, (int8_t)by});
									board[from_idx] = nullptr;

									// --- PLAYER PROMOTION ---
									if (board[to_idx]->getType() == PieceType::PAWN &&
											(by == 0 || by == 7)) {
										// Auto-promote to Queen
										board[to_idx] = std::make_unique<Queen>(
												g_state.turn, board[to_idx]->getPosition());
									}

									if (is_castling) {
										int r_from_x = (bx > g_state.selected_sq.x) ? 7 : 0;
										int r_to_x = (bx > g_state.selected_sq.x) ? 5 : 3;
										int r_from_idx = by * 8 + r_from_x;
										int r_to_idx = by * 8 + r_to_x;
										board[r_to_idx] = std::move(board[r_from_idx]);
										board[r_to_idx]->setPosition({(int8_t)r_to_x, (int8_t)by});
										board[r_from_idx] = nullptr;
									}

									g_state.en_passant_target = next_ep_target;
									g_state.turn = (g_state.turn == Color::WHITE) ? Color::BLACK :
																					Color::WHITE;
									checkGameState(board);
								}
								break;
							}
						}
						g_state.selected_sq = {-1, -1};
						g_state.valid_moves.clear();
					}
				}
			}
		}

		if (!g_state.in_menu && !g_state.game_over && g_state.vs_engine &&
				!g_state.engine_thinking) {
			if (g_state.turn != g_state.player_color) {
				g_state.engine_thinking = true;
				std::string fen = generateFEN(board, g_state.turn);
				g_state.stockfish.setPosition(fen);
				g_state.stockfish.go(10, 1000);
			}
		}

		if (g_state.vs_engine && g_state.engine_thinking) {
			auto move = g_state.stockfish.getBestMove();
			if (move) {
				std::string m = *move;
				std::println("DEBUG: Engine moved: {}", m);
				g_state.move_history.push_back(m);
				g_state.scroll_to_bottom = true;

				int fx = m[0] - 'a';
				int fy = 8 - (m[1] - '0');
				int tx = m[2] - 'a';
				int ty = 8 - (m[3] - '0');
				int from = fy * 8 + fx;
				int to = ty * 8 + tx;

				if (from >= 0 && from < 64 && to >= 0 && to < 64 && board[from]) {
					if (board[from]->getType() == PieceType::KING && std::abs(tx - fx) > 1) {
						int r_from_x = (tx > fx) ? 7 : 0;
						int r_to_x = (tx > fx) ? 5 : 3;
						int r_from_idx = fy * 8 + r_from_x;
						int r_to_idx = fy * 8 + r_to_x;
						board[r_to_idx] = std::move(board[r_from_idx]);
						board[r_to_idx]->setPosition({(int8_t)r_to_x, (int8_t)fy});
						board[r_from_idx] = nullptr;
					}

					BoardCoordinates next_ep_target = {-1, -1};
					if (board[from]->getType() == PieceType::PAWN && std::abs(ty - fy) == 2) {
						next_ep_target = {(int8_t)tx, (int8_t)((fy + ty) / 2)};
					}

					if (board[from]->getType() == PieceType::PAWN && tx != fx &&
							board[to] == nullptr) {
						int capture_idx = fy * 8 + tx;
						board[capture_idx] = nullptr;
					}

					board[to] = std::move(board[from]);
					board[to]->setPosition({(int8_t)tx, (int8_t)ty});
					board[from] = nullptr;

					// --- ENGINE PROMOTION ---
					if (m.length() == 5) {
						char promo = m[4];
						Color c = board[to]->getColor();
						BoardCoordinates pos = board[to]->getPosition();
						switch (promo) {
						case 'q':
							board[to] = std::make_unique<Queen>(c, pos);
							break;
						case 'r':
							board[to] = std::make_unique<Rook>(c, pos);
							break;
						case 'b':
							board[to] = std::make_unique<Bishop>(c, pos);
							break;
						case 'n':
							board[to] = std::make_unique<Knight>(c, pos);
							break;
						default:
							break;
						}
					}

					g_state.en_passant_target = next_ep_target;
					g_state.turn = g_state.player_color;
					checkGameState(board);
				} else {
					g_state.turn = g_state.player_color;
				}
				g_state.engine_thinking = false;
			}
		}

		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		int win_w, win_h;
		SDL_GetWindowSize(window, &win_w, &win_h);

		if (g_state.in_menu) {
			ImGui::SetNextWindowPos(
					ImVec2(win_w * 0.5f, win_h * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			ImGui::SetNextWindowSize(
					ImVec2(std::min(800.0f, win_w * 0.9f), std::min(700.0f, win_h * 0.9f)),
					ImGuiCond_Always);
			ImGui::Begin("Game Menu", nullptr, ImGuiWindowFlags_NoDecoration);
			ImGui::SetWindowFontScale(3.0f);
			float w = ImGui::GetWindowWidth();
			ImGui::SetCursorPosX((w - ImGui::CalcTextSize("CHESS").x) * 0.5f);
			ImGui::Text("CHESS");
			ImGui::SetWindowFontScale(2.0f);
			ImGui::Separator();

			static int mode = 0;
			ImGui::Text("Mode:");
			ImGui::SameLine();
			ImGui::RadioButton("PvP", &mode, 0);
			ImGui::SameLine();
			ImGui::RadioButton("PvE", &mode, 1);

			static int color_choice = 0;
			ImGui::Text("Color:");
			ImGui::SameLine();
			ImGui::RadioButton("White", &color_choice, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Black", &color_choice, 1);

			ImGui::SliderInt("Difficulty", &g_state.difficulty, 0, 20);

			if (ImGui::Button("START", ImVec2(-1, 80))) {
				g_state.vs_engine = (mode == 1);
				g_state.player_color = (color_choice == 0) ? Color::WHITE : Color::BLACK;
				resetBoard();
				if (g_state.vs_engine) {
					g_state.stockfish.start();
					g_state.stockfish.setSkillLevel(g_state.difficulty);
				} else
					g_state.stockfish.stop();
				g_state.in_menu = false;
			}
			ImGui::End();
		} else {
			ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(350, 0), ImGuiCond_Always);
			ImGui::Begin("Controls", nullptr,
					ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::SetWindowFontScale(1.5f);
			ImGui::TextWrapped("%s", g_state.status_msg.c_str());
			if (ImGui::Button("MENU", ImVec2(-1, 50))) {
				g_state.in_menu = true;
				g_state.stockfish.stop();
			}

			ImGui::Separator();
			ImGui::BeginChild("History", ImVec2(0, 200), true);
			for (size_t i = 0; i < g_state.move_history.size(); ++i) {
				if (i % 2 == 0)
					ImGui::Text("%d. %s", (int)(i / 2 + 1), g_state.move_history[i].c_str());
				else {
					ImGui::SameLine();
					ImGui::Text("%s", g_state.move_history[i].c_str());
				}
			}
			if (g_state.scroll_to_bottom)
				ImGui::SetScrollHereY(1.0f);
			g_state.scroll_to_bottom = false;
			ImGui::EndChild();
			ImGui::End();
		}

		SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
		SDL_RenderClear(renderer);
		drawBoardBackground();
		renderBoard();
		ImGui::Render();
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
		SDL_RenderPresent(renderer);
	}

	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
}

void App::drawBoardBackground() const {
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	auto board_size{static_cast<float>(std::min(h, w))};
	auto square_size{board_size / 8};
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			SDL_FRect square{j * square_size, i * square_size, square_size, square_size};
			if ((i + j) % 2 == 0)
				SDL_SetRenderDrawColor(renderer, 0, 180, 0, 255);
			else
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderFillRect(renderer, &square);
			if (g_state.selected_sq.x == j && g_state.selected_sq.y == i) {
				SDL_SetRenderDrawColor(renderer, 200, 255, 200, 100);
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
				SDL_RenderFillRect(renderer, &square);
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
			}
			for (auto& m : g_state.valid_moves) {
				if (m.x == j && m.y == i) {
					SDL_SetRenderDrawColor(renderer, 50, 255, 50, 128);
					SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
					SDL_FRect dot = {square.x + square.w / 3, square.y + square.h / 3, square.w / 3,
						square.h / 3};
					SDL_RenderFillRect(renderer, &dot);
					SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
				}
			}
		}
	}
}

void App::renderBoard() const {
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	float board_size = std::min(w, h);
	float square_size = board_size / 8;
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			auto& piece = board[y * 8 + x];
			if (piece) {
				char c = getPieceChar(piece);
				if (g_state.textures.count(c)) {
					SDL_FRect dest = {x * square_size, y * square_size, square_size, square_size};
					SDL_RenderTexture(renderer, g_state.textures[c], nullptr, &dest);
				} else {
					SDL_SetRenderDrawColor(
							renderer, piece->getColor() == Color::WHITE ? 200 : 50, 50, 50, 255);
					SDL_FRect rect = {x * square_size + 15, y * square_size + 15, square_size - 30,
						square_size - 30};
					SDL_RenderFillRect(renderer, &rect);
				}
			}
		}
	}
}

App::~App() {
	SDL_DestroyRenderer(this->renderer);
	SDL_DestroyWindow(this->window);
	SDL_Quit();
}