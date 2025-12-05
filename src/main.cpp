#include <cstdint>
#include <print>

//#include "imgui.h"
//#include "imgui_impl_sdl3.h"
//#include "imgui_impl_sdlrenderer3.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "app.hpp"

int32_t main(int32_t argc, char** argv) {
	for (size_t i = 0; i < static_cast<size_t>(argc); i++) {
		std::println("args: {}", argv[i]);
	}

	const App app{};
	app.run();

	return 0;
}

//
////// Setup SDL
////// [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
//SDL_InitFlags init_flags{SDL_INIT_VIDEO};
//if (!SDL_Init(init_flags)) {
//	std::println("Error: SDL_Init(): {}", SDL_GetError());
//	return 1;
//}
//
//// Create window with SDL_Renderer graphics context
//auto main_scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
//constexpr int32_t initial_window_width{800};
//constexpr int32_t initial_window_height{800};
//
//SDL_WindowFlags window_flags{SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY};
//auto* window{SDL_CreateWindow(
//		"Dear ImGui SDL3+SDL_Renderer example",
//		initial_window_width * static_cast<int32_t>(main_scale),
//		initial_window_height * static_cast<int32_t>(main_scale),
//		window_flags)};
//if (window == nullptr) {
//	std::println("Error: SDL_CreateWindow(): {}", SDL_GetError());
//	return 1;
//}
//auto* renderer{SDL_CreateRenderer(window, nullptr)};
//if (renderer == nullptr) {
//	SDL_Log("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
//	return 1;
//}
//// vsync, otherwise 100% CPU usage
//SDL_SetRenderVSync(renderer, 1);
//SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
//SDL_ShowWindow(window);
//
//constexpr struct {
//	float x{0.45f};
//	float y{0.55f};
//	float z{0.60f};
//	float w{1.00f};
//} clear_color;
//
//// load images
//auto* pawn_surface{IMG_Load("piece/bB.svg")};
//if (pawn_surface == nullptr) {
//	std::println("IMG_Load Error: {}", SDL_GetError());
//	SDL_Quit();
//	return 1;
//}
//auto* pawn_texture{SDL_CreateTextureFromSurface(renderer, pawn_surface)};
//if (pawn_texture == nullptr) {
//	std::println("SDL_CreateTextureFromSurface Error: {}", SDL_GetError());
//	SDL_Quit();
//	return 1;
//}
//SDL_SetTextureScaleMode(pawn_texture, SDL_SCALEMODE_NEAREST);
//
//// Main loop
//auto done{false};
//Uint64 previous_frame{0};
//int32_t window_width{initial_window_width};
//int32_t window_height{initial_window_height};
//while (!done) {
//	//// Poll and handle events (inputs, window resize, etc.)
//	//// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
//	//// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
//	//// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
//	//// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
//	//// [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
//	SDL_Event event{};
//	while (SDL_PollEvent(&event)) {
//		if (event.type == SDL_EVENT_QUIT) {
//			done = true;
//		}
//		if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
//				event.window.windowID == SDL_GetWindowID(window)) {
//			done = true;
//		}
//		if (event.type == SDL_EVENT_WINDOW_RESIZED) {
//			if (!SDL_GetWindowSize(window, &window_width, &window_height)) {
//				std::println("Error: SDL_GetWindowSize(): {}", SDL_GetError());
//				return 1;
//			}
//			SDL_Delay(10);
//		}
//	}
//
//	//// [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
//	if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
//		SDL_Delay(10);
//		continue;
//	}
//
//	// Rendering
//	//SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
//	SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
//	SDL_RenderClear(renderer);
//
//	//// draw board
//	//auto board_size{static_cast<float>(std::min(window_height, window_width))};
//	//auto square_size{board_size / 8};
//	//for (size_t i = 0; i < 8; i++) {
//	//	for (size_t j = 0; j < 8; j++) {
//	//		const SDL_FRect square{
//	//			.x = i * square_size,
//	//			.y = j * square_size,
//	//			.w = square_size,
//	//			.h = square_size};
//
//	//		if ((i + j) % 2 == 0) {
//	//			SDL_SetRenderDrawColor(renderer, 0xF0, 0xD9, 0xB5, SDL_ALPHA_OPAQUE);
//	//		} else {
//	//			SDL_SetRenderDrawColor(renderer, 0xB5, 0x88, 0x63, SDL_ALPHA_OPAQUE);
//	//		}
//
//	//		SDL_RenderFillRect(renderer, &square);
//	//		SDL_RenderTexture(renderer, pawn_texture, nullptr, &square);
//	//	}
//	//}
//	SDL_RenderPresent(renderer);
//
//	Uint64 this_frame{SDL_GetTicks()};
//	std::println("time delta: {}", this_frame - previous_frame);
//	previous_frame = this_frame;
//}
//
//// Cleanup
//// [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppQuit() function]
////
//SDL_DestroyRenderer(renderer);
//SDL_DestroyWindow(window);
//SDL_Quit();
