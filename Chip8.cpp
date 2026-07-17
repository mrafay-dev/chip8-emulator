#include "Chip8.hpp"
#include "opcodes.hpp"
#include <cstdint>
#include <fstream>
#include <chrono>
#include <random>
#include <cstring>
#include <optional>

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;
const unsigned int FONT_START_ADDRESS = 0x50;

uint8_t fontset[FONTSET_SIZE] = {  //each char is represented in hex here
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8()
	: randGen(std::chrono::system_clock::now().time_since_epoch().count())
	{	
		pc = START_ADDRESS;
		//load fonts into memory
		for (unsigned int i = 0; i<FONTSET_SIZE; ++i) {
			memory[FONT_START_ADDRESS + i] = fontset[i];
		}
		
		//init the RNG
		randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
		
		//function pointer tables
		
		//all the ones which are unique
		table[0x0] = &Chip8::Table0;
		table[0x1] = &Chip8::OP_1nnn;
		table[0x2] = &Chip8::OP_2nnn;
		table[0x3] = &Chip8::OP_3xkk;
		table[0x4] = &Chip8::OP_4xkk;
		table[0x5] = &Chip8::OP_5xy0;
		table[0x6] = &Chip8::OP_6xkk;
		table[0x7] = &Chip8::OP_7xkk;
		table[0x8] = &Chip8::Table8;  
		table[0x9] = &Chip8::OP_9xy0;
		table[0xA] = &Chip8::OP_Annn;
		table[0xB] = &Chip8::OP_Bnnn;
		table[0xC] = &Chip8::OP_Cxkk;
		table[0xD] = &Chip8::OP_Dxyn;
		table[0xE] = &Chip8::TableE;
		table[0xF] = &Chip8::TableF;
		
		//skip n/a table values
		for (size_t i = 0; i<= 0xE; i++) {
			table0[i] = &Chip8::OP_NULL;
			table8[i] = &Chip8::OP_NULL;
			tableE[i] = &Chip8::OP_NULL;
		}
		
		table0[0x0] = &Chip8::OP_00E0;
		table0[0xE] = &Chip8::OP_00EE;

		table8[0x0] = &Chip8::OP_8xy0;
		table8[0x1] = &Chip8::OP_8xy1;
		table8[0x2] = &Chip8::OP_8xy2;
		table8[0x3] = &Chip8::OP_8xy3;
		table8[0x4] = &Chip8::OP_8xy4;
		table8[0x5] = &Chip8::OP_8xy5;
		table8[0x6] = &Chip8::OP_8xy6;
		table8[0x7] = &Chip8::OP_8xy7;
		table8[0xE] = &Chip8::OP_8xyE;

		tableE[0x1] = &Chip8::OP_ExA1;
		tableE[0xE] = &Chip8::OP_Ex9E;
		
		for (size_t i = 0; i<= 0x65; i++) {
			tableF[i] = &Chip8::OP_NULL;
		}
		
		tableF[0x07] = &Chip8::OP_Fx07;
		tableF[0x0A] = &Chip8::OP_Fx0A;
		tableF[0x15] = &Chip8::OP_Fx15;
		tableF[0x18] = &Chip8::OP_Fx18;
		tableF[0x1E] = &Chip8::OP_Fx1E;
		tableF[0x29] = &Chip8::OP_Fx29;
		tableF[0x33] = &Chip8::OP_Fx33;
		tableF[0x55] = &Chip8::OP_Fx55;
		tableF[0x65] = &Chip8::OP_Fx65;
	}

void Chip8::LoadRom(char const* filename) {
	//ios::binary opens in binary mode. ate -> move pointer to the end. | means to do everything asked.
	std::ifstream file(filename, std::ios::binary | std::ios::ate);  
	
	if (file.is_open()) {  //finds size of file & allocates buffer to hold contents
		//tellg() - returns position of the file pointer
		//streampos - represents abs positions in a stream/file
		std::streampos size = file.tellg();
		
		char* buffer = new char[size]; 		//char better than int here as raw pointer
		
		//got to beg of file
		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();
		
		//load rom content into memory. starts at 0x200
		for (long i = 0; i<size; ++i) {
			memory[START_ADDRESS + i] = buffer[i]; // how does buffer get filled??
		}
		
		delete[] buffer; 
		
	}
}

void Chip8::Cycle(){
	//fetching
	//why +1 here????
	opcode = (memory[pc] << 8u) | memory[pc + 1];
	
	//increment pc
	pc += 2;
	
	//decode + execute
	((*this).*(table[(opcode & 0xF000u) >> 12u]))(); // gets the first digit of the opcode. then puts it through the function dispatch table
	
	if (delay_timer > 0) {
		--delay_timer;
	}
	
	if (sound_timer > 0) {
		--sound_timer;
	}
}

void Chip8::Table0() {
	((*this).*(table0[Opcode::n(opcode)]))();
}

void Chip8::Table8() {
	((*this).*(table8[Opcode::n(opcode)]))();
}

void Chip8::TableE() {
	((*this).*(tableE[Opcode::n(opcode)]))();
}

void Chip8::TableF() {
	((*this).*(tableF[opcode & 0x00FFu]))();
}


//null function
void Chip8::OP_NULL() {}

//clears display - sets buffer to 0
void Chip8::OP_00E0() {
	memset(display, 0, sizeof(display));
}

//return from subroutine
void Chip8::OP_00EE() {
	--stack_pointer;
	pc = stack[stack_pointer];
}

//jumps to location nnn (hence the &)
//no stack manip. because jump doesnt remember origin
void Chip8::OP_1nnn() {
	uint16_t address = Opcode::nnn(opcode);
	pc = address;
}

//call subroutine at 2nnn
void Chip8::OP_2nnn() {
	uint16_t address = Opcode::nnn(opcode);
	stack[stack_pointer] = pc;
	++stack_pointer;
	pc = address;
}

//skip next instruction of Vx == kk
void Chip8::OP_3xkk () {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t byte = Opcode::kk(opcode);
	
	if (registers[Vx] == byte) {
		pc += 2;
	}
}

//skip next instruction if Vx!=kk
void Chip8::OP_4xkk() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t byte = Opcode::kk(opcode);
	
	if (registers[Vx] != byte) {
		pc += 2;
	}
}

//skip next instruction if Vx == Vy
void Chip8::OP_5xy0() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t Vy = Opcode::y(opcode);
	
	if (registers[Vx] == registers[Vy]) {
		pc += 2;
	}
}

//set Vx = kk
void Chip8::OP_6xkk() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t byte = Opcode::kk(opcode);
	
	registers[Vx] = byte;
}

//set Vx += kk
void Chip8::OP_7xkk() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t byte = Opcode::kk(opcode);
	
	registers[Vx] += byte;
}

//vx = vy
void Chip8::OP_8xy0() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t Vy = Opcode::y(opcode);


	
	registers[Vx] = registers[Vy];
}

//Vx = Vx OR Vy
void Chip8::OP_8xy1() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t Vy = Opcode::y(opcode);

	
	registers[Vx] |= registers[Vy];
}

//Vx = Vx XOR Vy
void Chip8::OP_8xy3() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t Vy = Opcode::y(opcode);


	
	registers[Vx] ^= registers[Vy];
}

//adds vx and vy, checks for overflow
void Chip8::OP_8xy4() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t Vy = Opcode::y(opcode);


	
	uint16_t sum = registers[Vx] + registers[Vy];
	
	if (sum > 255u) {
		registers[0xF] = 1;  //sets the flag bit to 1
	} else {
		registers[0xF] = 0;
	}
	
	registers[Vx] = sum & 0xFFu;
}

//subtracts vx and vy
void Chip8::OP_8xy5() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t Vy = Opcode::y(opcode);


	
	if (registers[Vx] > registers[Vy]) {
		registers[0xF] = 1;  //sets the flag bit to 1
	} else {
		registers[0xF] = 0;
	}
	
	registers[Vx] -= registers[Vy];
}

//right shift and saves lsb in vf
void Chip8::OP_8xy6() {
	uint8_t Vx = Opcode::x(opcode);
	
	//saves the LSB in vf
	registers[0xF] = (registers[Vx] & 0x1u);
	registers[Vx] >>= 1;
}

//does whats on the tin
void Chip8::OP_8xy7() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t Vy = Opcode::y(opcode);


	
	if (registers[Vy] > registers[Vx]) {
		registers[0xF] = 1;  //sets the flag bit to 1
	} else {
		registers[0xF] = 0;
	}
	
	registers[Vx] = registers[Vy] - registers[Vx];
}

//left shift and stores msb in vf
void Chip8::OP_8xyE() {
	uint8_t Vx = Opcode::x(opcode);
	
	//saves msb in vf
	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;
	
	registers[Vx] <<= 1;
}

void Chip8::OP_8xy2()
{
    uint8_t Vx = Opcode::x(opcode);  //Opcode.x(opcode);
    uint8_t Vy = Opcode::y(opcode);

    registers[Vx] &= registers[Vy];
}

//skip next instruction if vx!=vy
void Chip8::OP_9xy0() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t Vy = Opcode::y(opcode);


	
	if (registers[Vx] != registers[Vy]) {
		pc += 2;
	}
}

//I = nnn
void Chip8::OP_Annn() {
	uint16_t address = Opcode::nnn(opcode);
	
	register_index = address;
}

//jump to nnn+v0
void Chip8::OP_Bnnn() {
	uint16_t address = Opcode::nnn(opcode);
	
	pc = registers[0] + address;
}

//vx = randbyte AND kk
void Chip8::OP_Cxkk() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t byte = Opcode::kk(opcode);
	
	registers[Vx] = randByte(randGen) & byte;
}

// display n-byte sprite at mem loc I at vx,vy. vf = collision
void Chip8::OP_Dxyn() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t Vy = Opcode::y(opcode);


	uint8_t height = Opcode::n(opcode);
	
	//wrap if beyond boundaries
	uint8_t xPos = registers[Vx] % VID_WIDTH;
	uint8_t yPos = registers[Vy] % VID_HEIGHT;
	
	registers[0xF] = 0; //what register is this?
	
	for (unsigned int row = 0; row < height; ++row) {
		uint8_t spriteByte = memory[register_index + row]; //??what does this do/
		
		for (unsigned int col = 0; col < 8; ++col) {
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			//figure out this
			uint32_t* screenPixel = &display[(yPos + row) * VID_WIDTH +(xPos + col)];
			
			//sprite pixel on
			if (spritePixel) {
				//screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					registers[0xF] = 1;
				}
				
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

//skip if key = Vx
void Chip8::OP_Ex9E() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t key = registers[Vx];
	
	if (input_keys[key]) {
		pc += 2;
	}
}

//skip if Vx key not pressed
void Chip8::OP_ExA1() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t key = registers[Vx];
	
	if (!input_keys[key]) {
		pc += 2;
	}
}

//Vx = delay timer val
void Chip8::OP_Fx07() {
	uint8_t Vx = Opcode::x(opcode);
	
	registers[Vx] = delay_timer;
}

//wait for key press & store in Vx
//decrement pc by 2 so it runs same instruction repeatedly
void Chip8::OP_Fx0A() {
	uint8_t Vx = Opcode::x(opcode);

	
	std::optional<uint8_t> pressed = std::nullopt;
	for (size_t key = 0; key<KEY_COUNT; ++key) {
		if(input_keys[key]) {
			pressed = static_cast<uint8_t>(key);
			break;
		}
	}

	if(pressed.has_value()) {
		registers[Vx] = pressed.value();
	} else {
		pc -= 2;
	}	
}	

//set delay timer to vx
void Chip8::OP_Fx15() {
	uint8_t Vx = Opcode::x(opcode);
	
	delay_timer= registers[Vx];
}

//sound timer to vx
void Chip8::OP_Fx18() {
	uint8_t Vx = Opcode::x(opcode);
	
	sound_timer = registers[Vx];
}

// I = I + vx
void Chip8::OP_Fx1E() {
	uint8_t Vx = Opcode::x(opcode);
	
	register_index += registers[Vx];
}

//I = location of sprite for Vx digit
void Chip8::OP_Fx29() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t digit = registers[Vx];
	
	register_index = FONT_START_ADDRESS + (5 * digit);
}

//stores bcd representation of Vx in I, I+1, I+2
void Chip8::OP_Fx33() {
	uint8_t Vx = Opcode::x(opcode);
	uint8_t value = registers[Vx];
	
	//units
	memory[register_index+2] = value % 10;
	value /= 10;
	
	//tens
	memory[register_index+1] = value % 10;
	value /= 10;
	
	//hundreds
	memory[register_index] = value % 10;
}

//stores registers v0 to vX in memory starting at I
void Chip8::OP_Fx55() {
	uint8_t Vx = Opcode::x(opcode);
	
	for (uint8_t i = 0; i <= Vx; ++i){
		memory[register_index+i] = registers[i];
	}
}

//read registers v0 to vx starting at location I
void Chip8::OP_Fx65() {
	uint8_t Vx = Opcode::x(opcode);
	
	for (uint8_t i = 0; i <= Vx; ++i){
		registers[i] = memory[register_index+i];
	}
}


