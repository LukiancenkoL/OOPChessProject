#include "piece.hpp"
#include "pawn.hpp"
#include "rook.hpp"
#include "knight.hpp"
#include "bishop.hpp"
#include "queen.hpp"
#include "king.hpp"

bool isValid(int x, int y) {
	return x >= 0 && x < 8 && y >= 0 && y < 8;
}

// --- Pawn ---
Pawn::Pawn(Color c, BoardCoordinates pos)
	: position(pos)
	, color(c)
	, has_moved(false) {
}
Color Pawn::getColor() const {
	return color;
}
PieceType Pawn::getType() const {
	return PieceType::PAWN;
}
BoardCoordinates Pawn::getPosition() const {
	return position;
}
void Pawn::setPosition(BoardCoordinates new_pos) {
	position = new_pos;
	has_moved = true;
}
bool Pawn::hasMoved() const {
	return has_moved;
}

std::vector<BoardCoordinates> Pawn::getPossibleMoves(const BoardArray& board) const {
	std::vector<BoardCoordinates> moves;
	int direction = (color == Color::WHITE) ? -1 : 1;
	int forward_y = position.y + direction;

	if (isValid(position.x, forward_y)) {
		int idx = forward_y * 8 + position.x;
		if (board[idx] == nullptr) {
			moves.push_back({position.x, (int8_t)forward_y});
			bool is_start = (color == Color::WHITE && position.y == 6) ||
					(color == Color::BLACK && position.y == 1);
			if (is_start) {
				int forward2_y = position.y + (direction * 2);
				if (isValid(position.x, forward2_y) &&
						board[forward2_y * 8 + position.x] == nullptr) {
					moves.push_back({position.x, (int8_t)forward2_y});
				}
			}
		}
	}
	int captureX[] = {position.x - 1, position.x + 1};
	for (int cx : captureX) {
		if (isValid(cx, forward_y)) {
			int idx = forward_y * 8 + cx;
			if (board[idx] != nullptr && board[idx]->getColor() != color) {
				moves.push_back({(int8_t)cx, (int8_t)forward_y});
			}
		}
	}
	return moves;
}

// --- Rook ---
Rook::Rook(Color c, BoardCoordinates pos)
	: position(pos)
	, color(c)
	, has_moved(false) {
}
Color Rook::getColor() const {
	return color;
}
PieceType Rook::getType() const {
	return PieceType::ROOK;
}
BoardCoordinates Rook::getPosition() const {
	return position;
}
void Rook::setPosition(BoardCoordinates new_pos) {
	position = new_pos;
	has_moved = true;
}
bool Rook::hasMoved() const {
	return has_moved;
}

std::vector<BoardCoordinates> Rook::getPossibleMoves(const BoardArray& board) const {
	std::vector<BoardCoordinates> moves;
	int directions[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
	for (auto& dir : directions) {
		for (int i = 1; i < 8; i++) {
			int nx = position.x + dir[0] * i;
			int ny = position.y + dir[1] * i;
			if (!isValid(nx, ny))
				break;
			int idx = ny * 8 + nx;
			if (board[idx] == nullptr) {
				moves.push_back({(int8_t)nx, (int8_t)ny});
			} else {
				if (board[idx]->getColor() != color)
					moves.push_back({(int8_t)nx, (int8_t)ny});
				break;
			}
		}
	}
	return moves;
}

// --- Knight ---
Knight::Knight(Color c, BoardCoordinates pos)
	: position(pos)
	, color(c) {
}
Color Knight::getColor() const {
	return color;
}
PieceType Knight::getType() const {
	return PieceType::KNIGHT;
}
BoardCoordinates Knight::getPosition() const {
	return position;
}
void Knight::setPosition(BoardCoordinates new_pos) {
	position = new_pos;
}

std::vector<BoardCoordinates> Knight::getPossibleMoves(const BoardArray& board) const {
	std::vector<BoardCoordinates> moves;
	int offsets[8][2] = {{1, 2}, {1, -2}, {-1, 2}, {-1, -2}, {2, 1}, {2, -1}, {-2, 1}, {-2, -1}};
	for (auto& off : offsets) {
		int nx = position.x + off[0];
		int ny = position.y + off[1];
		if (isValid(nx, ny)) {
			int idx = ny * 8 + nx;
			if (board[idx] == nullptr || board[idx]->getColor() != color) {
				moves.push_back({(int8_t)nx, (int8_t)ny});
			}
		}
	}
	return moves;
}

// --- Bishop ---
Bishop::Bishop(Color c, BoardCoordinates pos)
	: position(pos)
	, color(c) {
}
Color Bishop::getColor() const {
	return color;
}
PieceType Bishop::getType() const {
	return PieceType::BISHOP;
}
BoardCoordinates Bishop::getPosition() const {
	return position;
}
void Bishop::setPosition(BoardCoordinates new_pos) {
	position = new_pos;
}

std::vector<BoardCoordinates> Bishop::getPossibleMoves(const BoardArray& board) const {
	std::vector<BoardCoordinates> moves;
	int directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
	for (auto& dir : directions) {
		for (int i = 1; i < 8; i++) {
			int nx = position.x + dir[0] * i;
			int ny = position.y + dir[1] * i;
			if (!isValid(nx, ny))
				break;
			int idx = ny * 8 + nx;
			if (board[idx] == nullptr) {
				moves.push_back({(int8_t)nx, (int8_t)ny});
			} else {
				if (board[idx]->getColor() != color)
					moves.push_back({(int8_t)nx, (int8_t)ny});
				break;
			}
		}
	}
	return moves;
}

// --- Queen ---
Queen::Queen(Color c, BoardCoordinates pos)
	: position(pos)
	, color(c) {
}
Color Queen::getColor() const {
	return color;
}
PieceType Queen::getType() const {
	return PieceType::QUEEN;
}
BoardCoordinates Queen::getPosition() const {
	return position;
}
void Queen::setPosition(BoardCoordinates new_pos) {
	position = new_pos;
}

std::vector<BoardCoordinates> Queen::getPossibleMoves(const BoardArray& board) const {
	std::vector<BoardCoordinates> moves;
	int directions[8][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
	for (auto& dir : directions) {
		for (int i = 1; i < 8; i++) {
			int nx = position.x + dir[0] * i;
			int ny = position.y + dir[1] * i;
			if (!isValid(nx, ny))
				break;
			int idx = ny * 8 + nx;
			if (board[idx] == nullptr) {
				moves.push_back({(int8_t)nx, (int8_t)ny});
			} else {
				if (board[idx]->getColor() != color)
					moves.push_back({(int8_t)nx, (int8_t)ny});
				break;
			}
		}
	}
	return moves;
}

// --- King ---
King::King(Color c, BoardCoordinates pos)
	: position(pos)
	, color(c)
	, has_moved(false)
	, in_check(false) {
}
Color King::getColor() const {
	return color;
}
PieceType King::getType() const {
	return PieceType::KING;
}
BoardCoordinates King::getPosition() const {
	return position;
}
void King::setPosition(BoardCoordinates new_pos) {
	position = new_pos;
	has_moved = true;
}
bool King::hasMoved() const {
	return has_moved;
}

std::vector<BoardCoordinates> King::getPossibleMoves(const BoardArray& board) const {
	std::vector<BoardCoordinates> moves;
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			if (x == 0 && y == 0)
				continue;
			int nx = position.x + x;
			int ny = position.y + y;
			if (isValid(nx, ny)) {
				int idx = ny * 8 + nx;
				if (board[idx] == nullptr || board[idx]->getColor() != color) {
					moves.push_back({(int8_t)nx, (int8_t)ny});
				}
			}
		}
	}

	// CASTLING
	if (!has_moved) {
		int rook_k = position.y * 8 + 7;
		bool k_clear =
				(board[position.y * 8 + 5] == nullptr && board[position.y * 8 + 6] == nullptr);
		if (k_clear && board[rook_k] != nullptr && !board[rook_k]->hasMoved() &&
				board[rook_k]->getColor() == color) {
			moves.push_back({(int8_t)(position.x + 2), position.y});
		}

		int rook_q = position.y * 8 + 0;
		bool q_clear = (board[position.y * 8 + 1] == nullptr &&
				board[position.y * 8 + 2] == nullptr && board[position.y * 8 + 3] == nullptr);
		if (q_clear && board[rook_q] != nullptr && !board[rook_q]->hasMoved() &&
				board[rook_q]->getColor() == color) {
			moves.push_back({(int8_t)(position.x - 2), position.y});
		}
	}

	return moves;
}