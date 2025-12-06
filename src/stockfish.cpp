#include "stockfish.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <cstring>

Stockfish::Stockfish() {
}

Stockfish::~Stockfish() {
    stop();
}

bool Stockfish::start(const std::string& path) {
    if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0) return false;

    pid = fork();
    if (pid < 0) return false;

    if (pid == 0) {
        // Child process
        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        
        close(pipe_in[0]); close(pipe_in[1]);
        close(pipe_out[0]); close(pipe_out[1]);

        execlp(path.c_str(), path.c_str(), nullptr);
        std::exit(1);
    } else {
        close(pipe_in[0]);
        close(pipe_out[1]);
        
        int flags = fcntl(pipe_out[0], F_GETFL, 0);
        fcntl(pipe_out[0], F_SETFL, flags | O_NONBLOCK);
        
        writeCommand("uci");
        return true;
    }
}

void Stockfish::stop() {
    if (pid > 0) {
        writeCommand("quit");
        waitpid(pid, nullptr, 0);
        pid = -1;
    }
}

void Stockfish::setSkillLevel(int level) {
    if (level < 0) level = 0;
    if (level > 20) level = 20;
    writeCommand("setoption name Skill Level value " + std::to_string(level));
}

void Stockfish::writeCommand(const std::string& cmd) {
    if (pid == -1) return;
    std::string full_cmd = cmd + "\n";
    write(pipe_in[1], full_cmd.c_str(), full_cmd.length());
}

void Stockfish::setPosition(const std::string& fen, const std::vector<std::string>& moves) {
    accumulator.clear();
    std::string cmd = "position fen " + fen;
    if (!moves.empty()) {
        cmd += " moves";
        for (const auto& m : moves) cmd += " " + m;
    }
    writeCommand(cmd);
}

void Stockfish::go(int depth, int movetime_ms) {
    writeCommand("go depth " + std::to_string(depth) + " movetime " + std::to_string(movetime_ms));
}

std::optional<std::string> Stockfish::getBestMove() {
    char buffer[4096];
    ssize_t bytes = read(pipe_out[0], buffer, sizeof(buffer) - 1);
    
    if (bytes > 0) {
        buffer[bytes] = '\0';
        accumulator += buffer;
    }

    size_t pos = accumulator.find("bestmove");
    if (pos != std::string::npos) {
        size_t start = pos + 9;
        size_t end = accumulator.find(' ', start);
        if (end == std::string::npos) end = accumulator.find('\n', start);
        
        std::string moveStr = accumulator.substr(start, end - start);
        
        if (!moveStr.empty() && (moveStr.back() == '\r' || moveStr.back() == '\n')) {
            moveStr.pop_back();
        }
        
        accumulator.clear();
        return moveStr;
    }
    
    return std::nullopt;
}