#pragma once

#include <cstdint>
#include <vector>

enum class Color : uint8_t { WHITE, BLACK };

struct BoardCoordinates {
	uint8_t x;
	uint8_t y;
};

class Piece {
public:
	virtual std::vector<BoardCoordinates> getPossibleMoves() const& = 0;
	virtual Color getColor() const& = 0;
	virtual BoardCoordinates getPosition() const& = 0;
	virtual ~Piece() = default;
};
