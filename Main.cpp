//  g++ Main.cpp Chip8.cpp Platform.cpp glad.c -Iinclude -lSDL2 -ldl -o chip8

#include "Chip8.hpp"
#include "Platform.hpp"
#include <chrono>
#include <iostream>

int main(int argc, char** argv) {
	if (argc != 4) {
		std::cerr << "Usage: " << argv[0] << " <Scale> <Cycle ms> <ROM>\n";
		std::exit(EXIT_FAILURE);
	}
	
	int displayScale = std::stoi(argv[1]);
	const double CYCLE_MS  = std::stoi(argv[2]);
	char const* romFilename = argv[3];
	
	Platform platform("CHIP-8 Emulator", VID_WIDTH * displayScale, VID_HEIGHT * displayScale, VID_WIDTH, VID_HEIGHT);
	
	Chip8 chip8;
	chip8.LoadRom(romFilename);
	
	int videoPitch = sizeof(chip8.display[0])* VID_WIDTH;
	
	auto lastCycleTime = std::chrono::high_resolution_clock::now();
	bool quit = false;
	
	          // 2 ms per cycle (~500 Hz)
	double accumulator = 0.0;
	auto previous = std::chrono::high_resolution_clock::now();

	while (!quit) {
		auto current = std::chrono::high_resolution_clock::now();
		double delta = std::chrono::duration<double, std::milli>(current - previous).count();
		previous = current;
		accumulator += delta;

		// Poll input every frame
		quit = platform.ProcessInput(chip8.input_keys);

		// Run as many cycles as needed to catch up
		while (accumulator >= CYCLE_MS) {
			chip8.Cycle();
			accumulator -= CYCLE_MS;
		}

		// Render once per frame (or when dirty)
		platform.Update(chip8.display, videoPitch);
	}
	return 0;

}