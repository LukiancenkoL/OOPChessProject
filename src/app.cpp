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
#include <vector>

// Helper to get FEN character for piece
char getPieceChar(const std::unique_ptr<Piece>& p) {
    if (!p) return ' ';
    char c = '?';
    switch (p->getType()) {
        case PieceType::PAWN: c = 'p'; break;
        case PieceType::ROOK: c = 'r'; break;
        case PieceType::KNIGHT: c = 'n'; break;
        case PieceType::BISHOP: c = 'b'; break;
        case PieceType::QUEEN: c = 'q'; break;
        case PieceType::KING: c = 'k'; break;
        default: c = '?'; break;
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
    
    // --- NEW: History ---
    std::vector<std::string> move_history;
    bool scroll_to_bottom = false;
};

AppState g_state;

// Helper to convert coordinates to algebraic notation (e.g., 0,6 -> "a2")
std::string coordsToString(int x, int y) {
    char file = 'a' + x;
    char rank = '8' - y;
    return std::string{file, rank};
}

App::App() {
	std::println("DEBUG: Entering App constructor");

	if (!SDL_Init(App::init_flags)) {
		std::println("Error: SDL_Init(): {}", SDL_GetError());
		std::exit(EXIT_FAILURE);
	}

	auto main_scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
	this->window = SDL_CreateWindow(
			"Chess - C++ SDL3 ImGui",
			App::initial_window_width * static_cast<int32_t>(main_scale),
			App::initial_window_height * static_cast<int32_t>(main_scale),
			App::window_flags);
            
	if (this->window == nullptr) {
		std::println("Error: SDL_CreateWindow(): {}", SDL_GetError());
		std::exit(EXIT_FAILURE);
	}

	this->renderer = SDL_CreateRenderer(this->window, nullptr);
    if (this->renderer == nullptr) {
        std::println("Error: SDL_CreateRenderer: {}", SDL_GetError());
        std::exit(EXIT_FAILURE);
    }

	SDL_SetRenderVSync(this->renderer, 0);
    
    SDL_ShowWindow(this->window);
    std::println("DEBUG: Window Shown explicitly");
    
    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.5f); 

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    resetBoard();
    std::println("DEBUG: App constructor finished");
}

void App::resetBoard() {
    for(auto& p : board) p.reset();
    g_state.turn = Color::WHITE; 
    g_state.game_over = false;
    g_state.selected_sq = {-1, -1};
    g_state.valid_moves.clear();
    g_state.engine_thinking = false; 
    
    // Reset History
    g_state.move_history.clear();
    g_state.scroll_to_bottom = false;

    if (!g_state.in_menu) {
        g_state.status_msg = "White to move";
    }

    auto place = [&](int x, int y, Piece* p) {
        board[y * 8 + x].reset(p);
    };

    // Pawns
    for(int i=0; i<8; i++) {
        place(i, 1, new Pawn(Color::BLACK, {(int8_t)i, 1}));
        place(i, 6, new Pawn(Color::WHITE, {(int8_t)i, 6}));
    }

    // Others
    using C = Color;
    place(0, 0, new Rook(C::BLACK, {0,0})); place(7, 0, new Rook(C::BLACK, {7,0}));
    place(0, 7, new Rook(C::WHITE, {0,7})); place(7, 7, new Rook(C::WHITE, {7,7}));
    
    place(1, 0, new Knight(C::BLACK, {1,0})); place(6, 0, new Knight(C::BLACK, {6,0}));
    place(1, 7, new Knight(C::WHITE, {1,7})); place(6, 7, new Knight(C::WHITE, {6,7}));
    
    place(2, 0, new Bishop(C::BLACK, {2,0})); place(5, 0, new Bishop(C::BLACK, {5,0}));
    place(2, 7, new Bishop(C::WHITE, {2,7})); place(5, 7, new Bishop(C::WHITE, {5,7}));

    place(3, 0, new Queen(C::BLACK, {3,0})); place(4, 0, new King(C::BLACK, {4,0}));
    place(3, 7, new Queen(C::WHITE, {3,7})); place(4, 7, new King(C::WHITE, {4,7}));
}

void App::loadTextures() {
    std::println("DEBUG: Loading textures...");
    const char* pieces = "prnbqk";
    const char* colors = "wb";
    
    for (int c=0; c<2; c++) {
        for (int p=0; p<6; p++) {
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
            if (!surf) surf = IMG_Load(filename.c_str());

            if (surf) {
                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);
                char key = (c==0) ? std::toupper(pieces[p]) : pieces[p]; 
                g_state.textures[key] = tex;
                SDL_DestroySurface(surf);
            }
        }
    }
}

std::string generateFEN(const std::array<std::unique_ptr<Piece>, 64>& board, Color turn) {
    std::string fen = "";
    for (int y=0; y<8; y++) {
        int empty = 0;
        for (int x=0; x<8; x++) {
            auto& p = board[y*8 + x];
            if (!p) empty++;
            else {
                if (empty > 0) { fen += std::to_string(empty); empty = 0; }
                fen += getPieceChar(p);
            }
        }
        if (empty > 0) fen += std::to_string(empty);
        if (y < 7) fen += "/";
    }
    fen += (turn == Color::WHITE ? " w " : " b ");
    fen += "KQkq - 0 1"; 
    return fen;
}

// Check if square is attacked
bool isSquareAttacked(const std::array<std::unique_ptr<Piece>, 64>& board, BoardCoordinates sq, Color defenderColor) {
    for (const auto& p : board) {
        if (p && p->getColor() != defenderColor) {
            auto moves = p->getPossibleMoves(board);
            for (const auto& m : moves) {
                if (m.x == sq.x && m.y == sq.y) return true;
            }
        }
    }
    return false;
}

// Check if King is in Check
bool isKingInCheck(const std::array<std::unique_ptr<Piece>, 64>& board, Color kingColor) {
    BoardCoordinates kingPos = {-1, -1};
    for (const auto& p : board) {
        if (p && p->getType() == PieceType::KING && p->getColor() == kingColor) {
            kingPos = p->getPosition();
            break;
        }
    }
    if (kingPos.x == -1) return false; 
    return isSquareAttacked(board, kingPos, kingColor);
}

// Simulate move to check safety
bool isMoveSafe(std::array<std::unique_ptr<Piece>, 64>& board, int from_idx, int to_idx, Color turn) {
    std::unique_ptr<Piece> captured = std::move(board[to_idx]);
    board[to_idx] = std::move(board[from_idx]);
    
    BoardCoordinates old_pos = board[to_idx]->getPosition();
    BoardCoordinates new_pos = {(int8_t)(to_idx % 8), (int8_t)(to_idx / 8)};
    board[to_idx]->setPosition(new_pos);
    
    bool safe = !isKingInCheck(board, turn);
    
    // Undo
    board[to_idx]->setPosition(old_pos);
    board[from_idx] = std::move(board[to_idx]);
    board[to_idx] = std::move(captured);
    
    return safe;
}

// Check Game State (Mate/Stalemate)
void checkGameState(std::array<std::unique_ptr<Piece>, 64>& board) {
    bool in_check = isKingInCheck(board, g_state.turn);
    bool has_legal_moves = false;

    for (int i = 0; i < 64; ++i) {
        if (board[i] && board[i]->getColor() == g_state.turn) {
            auto moves = board[i]->getPossibleMoves(board);
            for (auto& m : moves) {
                int to_idx = m.y * 8 + m.x;
                if (isMoveSafe(board, i, to_idx, g_state.turn)) {
                    has_legal_moves = true;
                    break;
                }
            }
        }
        if (has_legal_moves) break;
    }

    if (!has_legal_moves) {
        g_state.game_over = true;
        if (in_check) {
            g_state.status_msg = (g_state.turn == Color::WHITE ? "Checkmate! Black Wins." : "Checkmate! White Wins.");
        } else {
            g_state.status_msg = "Stalemate! Draw.";
        }
    } else {
        if (in_check) {
            g_state.status_msg = "Check!";
        } else {
            g_state.status_msg = (g_state.turn == Color::WHITE ? "White to move" : "Black to move");
        }
    }
}

void App::run() {
    loadTextures();

	auto done{false};
    SDL_Event event{};

    std::println("DEBUG: Entering game loop");
	while (!done) {
		while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT) done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window)) done = true;
            
            if (g_state.in_menu || g_state.game_over) continue;

            if (!ImGui::GetIO().WantCaptureMouse && event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (g_state.vs_engine && g_state.engine_thinking) continue;

                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                float board_dim = std::min(w, h);
                float sq_size = board_dim / 8.0f;
                
                int mx = event.button.x;
                int my = event.button.y;
                
                if (mx < board_dim && my < board_dim) {
                    int bx = static_cast<int>(mx / sq_size);
                    int by = static_cast<int>(my / sq_size);
                    
                    if (g_state.selected_sq.x == -1) {
                        int idx = by*8 + bx;
                        if (board[idx] && board[idx]->getColor() == g_state.turn) {
                            if (g_state.vs_engine && g_state.turn != g_state.player_color) continue; 
                            g_state.selected_sq = {(int8_t)bx, (int8_t)by};
                            g_state.valid_moves = board[idx]->getPossibleMoves(board);
                        }
                    } else {
                        if (bx == g_state.selected_sq.x && by == g_state.selected_sq.y) {
                            g_state.selected_sq = {-1, -1};
                            g_state.valid_moves.clear();
                            continue;
                        }

                        for(auto& m : g_state.valid_moves) {
                            if (m.x == bx && m.y == by) {
                                int from_idx = g_state.selected_sq.y * 8 + g_state.selected_sq.x;
                                int to_idx = by * 8 + bx;
                                
                                // Validate safety before moving
                                if (isMoveSafe(board, from_idx, to_idx, g_state.turn)) {
                                    // Record Move for History
                                    std::string move_str = coordsToString(g_state.selected_sq.x, g_state.selected_sq.y) + coordsToString(bx, by);
                                    g_state.move_history.push_back(move_str);
                                    g_state.scroll_to_bottom = true;

                                    board[to_idx] = std::move(board[from_idx]);
                                    board[to_idx]->setPosition({(int8_t)bx, (int8_t)by});
                                    board[from_idx] = nullptr;
                                    
                                    g_state.turn = (g_state.turn == Color::WHITE) ? Color::BLACK : Color::WHITE;
                                    checkGameState(board); 
                                } else {
                                    std::println("Illegal Move: King is in check!");
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
        
        // Stockfish Engine Logic
        if (!g_state.in_menu && !g_state.game_over && g_state.vs_engine && !g_state.engine_thinking) {
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
                
                // Record History
                g_state.move_history.push_back(m);
                g_state.scroll_to_bottom = true;

                int fx = m[0] - 'a'; 
                int fy = 8 - (m[1] - '0'); 
                int tx = m[2] - 'a';
                int ty = 8 - (m[3] - '0');
                int from = fy*8 + fx;
                int to = ty*8 + tx;
                
                if (from >= 0 && from < 64 && to >= 0 && to < 64 && board[from]) {
                    board[to] = std::move(board[from]);
                    board[to]->setPosition({(int8_t)tx, (int8_t)ty});
                    board[from] = nullptr;
                    
                    g_state.turn = g_state.player_color;
                    checkGameState(board);
                } else {
                    std::println("ERROR: Engine attempted invalid move!");
                    g_state.turn = g_state.player_color;
                }
                g_state.engine_thinking = false;
            }
        }

        // GUI Frame Start
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        
        // --- UI ---
        int win_w, win_h;
        SDL_GetWindowSize(window, &win_w, &win_h);

        if (g_state.in_menu) {
            ImGui::SetNextWindowPos(ImVec2(win_w * 0.5f, win_h * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            float menu_w = std::min(800.0f, (float)win_w * 0.95f);
            float menu_h = std::min(700.0f, (float)win_h * 0.95f);
            ImGui::SetNextWindowSize(ImVec2(menu_w, menu_h), ImGuiCond_Always);
            
            ImGui::Begin("Game Menu", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            
            ImGui::SetWindowFontScale(3.0f);
            const char* title = "CHESS";
            float title_w = ImGui::CalcTextSize(title).x;
            ImGui::SetCursorPosX((menu_w - title_w) * 0.5f);
            ImGui::Text("%s", title);
            
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
            
            ImGui::SetWindowFontScale(2.0f);
            static int mode = 0;
            ImGui::Text("Game Mode:");
            ImGui::RadioButton("Player vs Player", &mode, 0);
            ImGui::RadioButton("Player vs Stockfish", &mode, 1);
            
            ImGui::Spacing();
            static int color_choice = 0;
            ImGui::Text("Your Color (vs Engine):");
            ImGui::RadioButton("White", &color_choice, 0);
            ImGui::RadioButton("Black", &color_choice, 1);
            
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
            
            ImGui::Text("Engine Power: %d", g_state.difficulty);
            ImGui::PushItemWidth(-1);
            ImGui::SliderInt("##difficulty", &g_state.difficulty, 0, 20);
            ImGui::PopItemWidth();
            
            ImGui::Spacing(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();
            
            if (ImGui::Button("START GAME", ImVec2(-1, 80))) {
                g_state.vs_engine = (mode == 1);
                g_state.player_color = (color_choice == 0) ? Color::WHITE : Color::BLACK;
                
                resetBoard(); 
                
                if (g_state.vs_engine) {
                    g_state.stockfish.start();
                    g_state.stockfish.setSkillLevel(g_state.difficulty);
                } else {
                    g_state.stockfish.stop();
                }
                
                g_state.in_menu = false;
            }
            ImGui::SetWindowFontScale(1.0f);
            ImGui::End();

        } else {
            // --- IN-GAME HUD ---
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(350, 0), ImGuiCond_Always); 
            
            ImGui::Begin("Chess Control", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::SetWindowFontScale(1.5f);
            
            ImGui::TextWrapped("Status: %s", g_state.status_msg.c_str());
            ImGui::Separator();
            
            if (g_state.game_over) {
                ImGui::TextColored(ImVec4(1, 0.2f, 0.2f, 1), "GAME OVER");
                if (ImGui::Button("NEW GAME", ImVec2(-1, 50))) {
                    g_state.in_menu = true;
                }
            } else {
                if (g_state.vs_engine) {
                    ImGui::Text("You: %s", (g_state.player_color == Color::WHITE ? "White" : "Black"));
                    ImGui::Text("Engine Lvl: %d", g_state.difficulty);
                } else {
                    ImGui::Text("Mode: PvP");
                }
                
                ImGui::Spacing();
                if (ImGui::Button("MENU", ImVec2(-1, 50))) {
                    g_state.in_menu = true;
                    g_state.stockfish.stop();
                }
            }
            
            // --- HISTORY ---
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
            ImGui::Text("History:");
            ImGui::BeginChild("HistoryRegion", ImVec2(0, 200), true);
            
            for (size_t i = 0; i < g_state.move_history.size(); ++i) {
                if (i % 2 == 0) {
                    // Start of a pair: "1. e2e4"
                    ImGui::Text("%d. %s", (int)(i/2 + 1), g_state.move_history[i].c_str());
                } else {
                    // Second move: "... e7e5" on the same line
                    ImGui::SameLine();
                    ImGui::Text("%s", g_state.move_history[i].c_str());
                }
            }
            
            if (g_state.scroll_to_bottom) {
                ImGui::SetScrollHereY(1.0f);
                g_state.scroll_to_bottom = false;
            }
            ImGui::EndChild();

            ImGui::End();
        }

        // Rendering
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
			
            if ((i + j) % 2 == 0) {
                 SDL_SetRenderDrawColor(renderer, 0, 180, 0, 255); 
            } else {
                 SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
            }
            
			SDL_RenderFillRect(renderer, &square);
            
            if (g_state.selected_sq.x == j && g_state.selected_sq.y == i) {
                SDL_SetRenderDrawColor(renderer, 200, 255, 200, 100);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_RenderFillRect(renderer, &square);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            }
            
            for(auto& m : g_state.valid_moves) {
                if (m.x == j && m.y == i) {
                    SDL_SetRenderDrawColor(renderer, 50, 255, 50, 128);
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_FRect dot = {square.x + square.w/3, square.y + square.h/3, square.w/3, square.h/3};
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
                    SDL_SetRenderDrawColor(renderer, piece->getColor()==Color::WHITE?200:50, 50, 50, 255);
                    SDL_FRect rect = {x * square_size + 15, y * square_size + 15, square_size-30, square_size-30};
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