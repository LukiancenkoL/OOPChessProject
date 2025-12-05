#pragma once

#include "piece.hpp"
class Rook : public Piece {
public:
	std::vector<BoardCoordinates> getPossibleMoves() const& override;
	Color getColor() const& override;
	BoardCoordinates getPosition() const& override;

private:
	BoardCoordinates position;
	Color color;
	bool has_moved;
};
