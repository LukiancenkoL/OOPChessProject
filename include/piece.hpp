#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <array>

enum class Color : uint8_t { WHITE, BLACK };
enum class PieceType : uint8_t { PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING };

struct BoardCoordinates {
	int8_t x;
	int8_t y;
    
    bool operator==(const BoardCoordinates& other) const {
        return x == other.x && y == other.y;
    }
};

class Piece {
public:
    using BoardArray = std::array<std::unique_ptr<Piece>, 64>;
	virtual std::vector<BoardCoordinates> getPossibleMoves(const BoardArray& board) const = 0;
    
	virtual Color getColor() const = 0;
    virtual PieceType getType() const = 0;
	virtual BoardCoordinates getPosition() const = 0;
    virtual void setPosition(BoardCoordinates new_pos) = 0;
    
	virtual ~Piece() = default;
};