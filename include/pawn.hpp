#pragma once
#include "piece.hpp"

class Pawn : public Piece {
public:
    Pawn(Color c, BoardCoordinates pos);
	std::vector<BoardCoordinates> getPossibleMoves(const BoardArray& board) const override;
	Color getColor() const override;
    PieceType getType() const override;
	BoardCoordinates getPosition() const override;
    void setPosition(BoardCoordinates new_pos) override;
	bool hasMoved() const override;

private:
	BoardCoordinates position;
	Color color;
	bool has_moved;
	bool can_promote;
};