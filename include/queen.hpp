#pragma once
#include "piece.hpp"

class Queen : public Piece {
public:
    Queen(Color c, BoardCoordinates pos);
	std::vector<BoardCoordinates> getPossibleMoves(const BoardArray& board) const override;
	Color getColor() const override;
    PieceType getType() const override;
	BoardCoordinates getPosition() const override;
    void setPosition(BoardCoordinates new_pos) override;

private:
	BoardCoordinates position;
	Color color;
};