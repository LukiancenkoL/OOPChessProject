#include <cstdint>
#include <print>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "app.hpp"

int32_t main(int32_t argc, char** argv) {
	for (size_t i = 0; i < static_cast<size_t>(argc); i++) {
		std::println("args: {}", argv[i]);
	}

	App app{};
	app.run();

	return 0;
}
