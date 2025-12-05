#include "app.hpp"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <cstdlib>
#include <print>

App::App() {
	std::println("app constructor");
	if (!SDL_Init(App::init_flags)) {
		std::println("Error: SDL_Init(): {}", SDL_GetError());
		std::exit(EXIT_FAILURE);
	}

	auto main_scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
	this->window = SDL_CreateWindow(
			"Dear ImGui SDL3+SDL_Renderer example",
			App::initial_window_width * static_cast<int32_t>(main_scale),
			App::initial_window_height * static_cast<int32_t>(main_scale),
			App::window_flags);
	if (App::window == nullptr) {
		std::println("Error: SDL_CreateWindow(): {}", SDL_GetError());
		std::exit(EXIT_FAILURE);
	}
	this->renderer = SDL_CreateRenderer(this->window, nullptr);
	if (this->renderer == nullptr) {
		SDL_Log("Error: SDL_Createthis->renderer(): %s\n", SDL_GetError());
		std::exit(EXIT_FAILURE);
	}
	// vsync, otherwise 100% CPU usage
	SDL_SetRenderVSync(this->renderer, 1);
	SDL_SetWindowPosition(this->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_ShowWindow(this->window);
}

void App::run(this const App& self) {
	std::println("app run");
	self.drawBoardBackground();

	// load images
	auto* pawn_surface{IMG_Load("piece/bB.svg")};
	if (pawn_surface == nullptr) {
		std::println("IMG_Load Error: {}", SDL_GetError());
		SDL_Quit();
		return;
	}
	auto* pawn_texture{SDL_CreateTextureFromSurface(self.renderer, pawn_surface)};
	if (pawn_texture == nullptr) {
		std::println("SDL_CreateTextureFromSurface Error: {}", SDL_GetError());
		SDL_Quit();
		return;
	}
	SDL_SetTextureScaleMode(pawn_texture, SDL_SCALEMODE_NEAREST);

	// Main loop
	auto done{false};
	Uint64 previous_frame{0};
	auto window_width{self.initial_window_width};
	auto window_height{self.initial_window_height};
	while (!done) {
		//// Poll and handle events (inputs, window resize, etc.)
		//// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		//// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		//// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		//// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		//// [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
		SDL_Event event{};
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				done = true;
				break;
			}
			if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
					event.window.windowID == SDL_GetWindowID(self.window)) {
				done = true;
			}
			if (event.type == SDL_EVENT_WINDOW_RESIZED) {
				if (!SDL_GetWindowSize(self.window, &window_width, &window_height)) {
					std::println("Error: SDL_GetWindowSize(): {}", SDL_GetError());
					return;
				}
				SDL_Delay(10);
			}
		}

		//// [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
		if (SDL_GetWindowFlags(self.window) & SDL_WINDOW_MINIMIZED) {
			SDL_Delay(10);
			continue;
		}

		constexpr struct {
			float x{0.45f};
			float y{0.55f};
			float z{0.60f};
			float w{1.00f};
		} clear_color;

		// Rendering
		//SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		SDL_SetRenderDrawColorFloat(self.renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		SDL_RenderClear(self.renderer);

		self.drawBoardBackground();
		auto board_size{static_cast<float>(std::min(window_height, window_width))};
		auto square_size{board_size / 8};
		for (size_t i = 0; i < 8; i++) {
			for (size_t j = 0; j < 8; j++) {
				const SDL_FRect square{
					.x = j * square_size,
					.y = i * square_size,
					.w = square_size,
					.h = square_size};
				if (i == 0) {
					SDL_RenderTexture(self.renderer, pawn_texture, nullptr, &square);
				}
				if (i == 0 && j) {
				}
			}
		}
		SDL_RenderPresent(self.renderer);

		auto this_frame{SDL_GetTicks()};
		std::println("time delta: {}", this_frame - previous_frame);
		previous_frame = this_frame;
	}
}

//void App::renderBoard(this const App& self) {}

void App::drawBoardBackground(this const App& self) {
	std::println("drawBoard");
	auto window_width{0};
	auto window_height{0};
	if (!SDL_GetWindowSize(self.window, &window_width, &window_height)) {
		std::println("Error: SDL_GetWindowSize(): {}", SDL_GetError());
		std::exit(EXIT_FAILURE);
	}

	auto board_size{static_cast<float>(std::min(window_height, window_width))};
	auto square_size{board_size / 8};
	for (size_t i = 0; i < 8; i++) {
		for (size_t j = 0; j < 8; j++) {
			const SDL_FRect square{
				.x = j * square_size,
				.y = i * square_size,
				.w = square_size,
				.h = square_size};

			if ((i + j) % 2 == 0) {
				SDL_SetRenderDrawColor(self.renderer, 0xF0, 0xD9, 0xB5, SDL_ALPHA_OPAQUE);
			} else {
				SDL_SetRenderDrawColor(self.renderer, 0xB5, 0x88, 0x63, SDL_ALPHA_OPAQUE);
			}
			SDL_RenderFillRect(self.renderer, &square);
		}
	}
}

App::~App() {
	std::println("app destructor");
	SDL_DestroyRenderer(this->renderer);
	SDL_DestroyWindow(this->window);
	SDL_Quit();
}
