#pragma once
#include "piece.hpp"

class Queen : public Piece {
public:
	std::vector<BoardCoordinates> getPossibleMoves() const& override;
	Color getColor() const& override;
	BoardCoordinates getPosition() const& override;

private:
	BoardCoordinates position;
	Color color;
};

