#pragma once

#include <string>
#include <vector>
#include <optional>

class Stockfish {
public:
    Stockfish();
    ~Stockfish();

    bool start(const std::string& path = "stockfish");
    void stop();
    void setSkillLevel(int level);
    void setPosition(const std::string& fen, const std::vector<std::string>& moves = {});
    void go(int depth = 10, int movetime_ms = 1000);
    std::optional<std::string> getBestMove();

private:
    void writeCommand(const std::string& cmd);

    int pipe_in[2];
    int pipe_out[2];
    int pid = -1;
    std::string accumulator;
};