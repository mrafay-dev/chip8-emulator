//  g++ Main.cpp Chip8.cpp Platform.cpp glad.c -Iinclude -lSDL2 -ldl -o chip8

#include "Chip8.hpp"
#include "Platform.hpp"
#include <chrono>
#include <iostream>

int main(int argc, char** argv) {
	if (argc != 4) {
		std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
		std::exit(EXIT_FAILURE);
	}
	
	int displayScale = std::stoi(argv[1]);
	int cycleDelay = std::stoi(argv[2]);
	char const* romFilename = argv[3];
	
	Platform platform("CHIP-8 Emulator", VID_WIDTH * displayScale, VID_HEIGHT * displayScale, VID_WIDTH, VID_HEIGHT);
	
	Chip8 chip8;
	chip8.LoadRom(romFilename);
	
	int videoPitch = sizeof(chip8.display[0])* VID_WIDTH;
	
	auto lastCycleTime = std::chrono::high_resolution_clock::now();
	bool quit = false;
	
	while (!quit) {
		quit = platform.ProcessInput(chip8.input_keys);
		
		auto currentTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();
		
		if (dt > cycleDelay) {
			lastCycleTime = currentTime;
			
			chip8.Cycle();
			
			platform.Update(chip8.display, videoPitch);
		}
	}
	return 0;

}