#pragma once

#include "piece.hpp"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <array>
#include <memory>

class App {
public:
	explicit App();
	explicit App(const App& other) = delete;
	App& operator=(const App& other) = delete;
	App(App&& other) noexcept = delete;
	App& operator=(App&& other) noexcept = delete;
	~App();

	void run(this const App& self);

private:
	void drawBoardBackground(this const App& self);
	void renderBoard(this const App& self);

private:
	static constexpr SDL_InitFlags init_flags{SDL_INIT_VIDEO};
	static constexpr SDL_WindowFlags window_flags{SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY};
	static constexpr auto initial_window_width{800};
	static constexpr auto initial_window_height{800};

	SDL_Window* window{};
	SDL_Renderer* renderer{};

	std::array<std::unique_ptr<Piece>, 64> board;
};
