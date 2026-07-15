#pragma once

#include <cstdint>
#include <random>


constexpr unsigned MEMORY_SIZE = 4096;
constexpr unsigned REGISTER_COUNT = 16;
constexpr unsigned STACK_SIZE = 16;
constexpr unsigned KEY_COUNT = 16;
constexpr unsigned VID_HEIGHT = 32;
constexpr unsigned VID_WIDTH = 64;

class Chip8 {
public:
	Chip8();
	void LoadRom(char const* filename);
	void Cycle();
	
	uint32_t display[VID_HEIGHT * VID_WIDTH]{};		//64 x 32 display
	uint8_t input_keys[KEY_COUNT]{};	

private:
	std::default_random_engine randGen;
	std::uniform_int_distribution<uint8_t> randByte;
	
	//null func
	void OP_NULL();
	
	// CLS
	void OP_00E0();

	// RET
	void OP_00EE();

	// JP address
	void OP_1nnn();

	// CALL address
	void OP_2nnn();

	// SE Vx, byte
	void OP_3xkk();

	// SNE Vx, byte
	void OP_4xkk();

	// SE Vx, Vy
	void OP_5xy0();

	// LD Vx, byte
	void OP_6xkk();

	// ADD Vx, byte
	void OP_7xkk();

	// LD Vx, Vy
	void OP_8xy0();

	// OR Vx, Vy
	void OP_8xy1();

	// AND Vx, Vy
	void OP_8xy2();

	// XOR Vx, Vy
	void OP_8xy3();

	// ADD Vx, Vy
	void OP_8xy4();

	// SUB Vx, Vy
	void OP_8xy5();

	// SHR Vx
	void OP_8xy6();

	// SUBN Vx, Vy
	void OP_8xy7();

	// SHL Vx
	void OP_8xyE();

	// SNE Vx, Vy
	void OP_9xy0();

	// LD I, address
	void OP_Annn();

	// JP V0, address
	void OP_Bnnn();

	// RND Vx, byte
	void OP_Cxkk();

	// DRW Vx, Vy, height
	void OP_Dxyn();

	// SKP Vx
	void OP_Ex9E();

	// SKNP Vx
	void OP_ExA1();

	// LD Vx, DT
	void OP_Fx07();

	// LD Vx, K
	void OP_Fx0A();

	// LD DT, Vx
	void OP_Fx15();

	// LD ST, Vx
	void OP_Fx18();

	// ADD I, Vx
	void OP_Fx1E();

	// LD F, Vx
	void OP_Fx29();

	// LD B, Vx
	void OP_Fx33();

	// LD [I], Vx
	void OP_Fx55();

	// LD Vx, [I]
	void OP_Fx65();
	
	uint8_t registers[REGISTER_COUNT]{}; 		//16 8 bit registers
	uint8_t memory[MEMORY_SIZE]{};			//4096 bytes of memory
	uint16_t register_index{};		
	uint16_t pc{};
	uint16_t stack[STACK_SIZE]{}; 			//tracks which stack we're on
	uint8_t stack_pointer{};
	uint8_t delay_timer{}; 		
	uint8_t sound_timer{};
	uint16_t opcode{};
	
	void Table0();
	void Table8();
	void TableE();
	void TableF();
	
	typedef void (Chip8::*Chip8Func)();
	Chip8Func table[0xF + 1];
	Chip8Func table0[0xE + 1];
	Chip8Func table8[0xE + 1];
	Chip8Func tableE[0xE + 1];
	Chip8Func tableF[0x65 + 1];

};