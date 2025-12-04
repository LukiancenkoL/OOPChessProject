#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdio.h>
#include <print>


#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

int32_t main(int32_t argc, char** argv) {
	for (size_t i = 0; i < static_cast<size_t>(argc); i++) {
		std::println("args: {}", argv[i]);
	}

	// Setup SDL
	// [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
	SDL_InitFlags flags{SDL_INIT_VIDEO};
	if (!SDL_Init(flags)) {
		std::println("Error: SDL_Init(): {}", SDL_GetError());
		return 1;
	}

	// Create window with SDL_Renderer graphics context
	auto main_scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
	constexpr int32_t initial_window_width{800};
	constexpr int32_t initial_window_height{800};

	SDL_WindowFlags window_flags{SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY};
	auto* window{SDL_CreateWindow(
			"Dear ImGui SDL3+SDL_Renderer example",
			initial_window_width * static_cast<int32_t>(main_scale),
			initial_window_height * static_cast<int32_t>(main_scale),
			window_flags)};
	if (window == nullptr) {
		std::println("Error: SDL_CreateWindow(): {}", SDL_GetError());
		return 1;
	}
	auto* renderer{SDL_CreateRenderer(window, nullptr)};
	if (renderer == nullptr) {
		SDL_Log("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
		return 1;
	}
	// vsync, otherwise 100% CPU usage
	SDL_SetRenderVSync(renderer, 1);
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_ShowWindow(window);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto& io{ImGui::GetIO()};
	//(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	ImGui::StyleColorsLight();

	// Setup scaling
	auto& style{ImGui::GetStyle()};
	style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	//style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer3_Init(renderer);

	// Our state
	auto show_demo_window{false};
	auto show_another_window{false};
	auto clear_color{ImVec4(0.45f, 0.55f, 0.60f, 1.00f)};

	// load images
	//auto pawn{IMG_Load("../piece/bP.svg")};

	// Main loop
    auto done{false};
	Uint64 previous_frame{0};
	int32_t window_width{initial_window_width};
	int32_t window_height{initial_window_height};
	while (!done) {
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		// [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
		SDL_Event event{};
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT) {
				done = true;
			}
			if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
					event.window.windowID == SDL_GetWindowID(window)) {
				done = true;
			}
			if (event.type == SDL_EVENT_WINDOW_RESIZED){
				if (!SDL_GetWindowSize(window, &window_width, &window_height)) {
					std::println("Error: SDL_GetWindowSize(): {}", SDL_GetError());
					return 1;
				}
			}
		}

		// [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
		if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
			SDL_Delay(10);
			continue;
		}

		// Start the Dear ImGui frame
		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window) {
			ImGui::ShowDemoWindow(&show_demo_window);
		}

		//// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		//{
		//	//static float f = 0.0f;
		//	//static int counter = 0;

		//	//ImGui::Begin(
		//	//		"Hello, world!"); // Create a window called "Hello, world!" and append into it.

		//	//ImGui::Text(
		//	//		"This is some useful text."); // Display some text (you can use a format strings too)
		//	//ImGui::Checkbox("Demo Window",
		//	//		&show_demo_window); // Edit bools storing our window open/close state
		//	//ImGui::Checkbox("Another Window", &show_another_window);

		//	//ImGui::SliderFloat(
		//	//		"float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
		//	//ImGui::ColorEdit3(
		//	//		"clear color", (float*)&clear_color); // Edit 3 floats representing a color

		//	//if (ImGui::Button(
		//	//			"Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
		//	//	counter++;
		//	//ImGui::SameLine();
		//	//ImGui::Text("counter = %d", counter);

		//	//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate,
		//	//		io.Framerate);
		//	//ImGui::End();
		//}

		// 3. Show another simple window.
		if (show_another_window) {
			ImGui::Begin("Another Window",
					&show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me")) {
				show_another_window = false;
			}
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		SDL_RenderClear(renderer);

		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
		// draw board
		auto board_size{static_cast<float>(std::min(window_height, window_width))};
		auto square_size{board_size / 8};
		for (size_t i = 0; i < 8; i++) {
			for (size_t j = 0; j < 8; j++) {
				const SDL_FRect square{
					.x = i * square_size,
					.y = j * square_size,
					.w = square_size,
					.h = square_size};

				if ((i + j) % 2 == 0) {
					SDL_SetRenderDrawColor(renderer, 0xF0, 0xD9, 0xB5, SDL_ALPHA_OPAQUE);
				} else {
					SDL_SetRenderDrawColor(renderer, 0xB5, 0x88, 0x63, SDL_ALPHA_OPAQUE);
				}
				if (i == 0) {
					//SDL_RenderTexture(renderer, pawn, nullptr, &square);
				}
				SDL_RenderFillRect(renderer, &square);
			}
		}
		SDL_RenderPresent(renderer);

		Uint64 this_frame{SDL_GetTicks()};
		std::println("time delta: {}", this_frame - previous_frame);
		previous_frame = this_frame;
	}

	// Cleanup
	// [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppQuit() function]
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
