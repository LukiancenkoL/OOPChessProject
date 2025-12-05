#pragma once

#include "piece.hpp"
class Pawn : public Piece {
public:
	std::vector<BoardCoordinates> getPossibleMoves() const& override;
	Color getColor() const& override;
	BoardCoordinates getPosition() const& override;

private:
	BoardCoordinates position;
	Color color;
	bool has_moved;
	bool can_promote;
};
