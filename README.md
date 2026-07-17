# The main Cycle

The video scale, cycle refresh rate in ms and ROM is entered by the user into the program. 

The ```LoadROM``` function is then called to read the game we want to play into memory, starting from memory location 0x200.

While the user doesn't quit, each cpu cycle, the input key is polled, and any missed frames are accumulated and processed before moving on to the next cycle.

The screen is only updated if once per frame.

The opcodes look like ```OP_8xy6``` for example. Because the hardware that Chip8 ran on was 1 byte, and the opcode is 2 bytes, the memory must hold 2 locations for one opcode. This means that the program_counter should incremenet by 2 to get to the next instruction. 

memory[pc] holds the most sig. byte of the opcode, and memory[pc + 1] holds the least sig. byte of the opcode. The instruction is then decoded as such: 

```cpp
opcode = (memory[pc] << 8u) | memory[pc + 1];
```

There are also delay and sound timers, which are used by the games we emulate, to make pauses and sounds. 

There are 16 registers which hold 1 byte which the program might need. The last register is a flag register. The register index variable accesses these values. 

There is also a 16 level stack, which is referenced using the stack pointer. An exception is the ```Jump``` opcode, which doesn't track the origin level, and jumps to the required stack level.


# Function Dispatch Table: 

The Chip8 emulator uses a function dispatch table to decode and execute cycles.

A function pointer type is defined as:

```cpp
typedef void (Chip8::*Chip8Func)();
```

Which is a pointer to a function in the Chip8 member function. All opcodes handlers are then defined as such:

```cpp
void Chip8::OP_8xy6();
```

These functions are then stored and called using a dispatch table. 

The main dispatch table is:

```cpp
	Chip8Func table[0xF + 1];
```
where each index is a function in the Chip8 class. Then each index stores the address of the function as such:

```cpp
table[0x1] = &Chip8::OP_1nnn;
```

Where there are repeated inital digits in the opcode, such as 8, we can do:

```cpp
table[0x8] = &Chip8::Table8;  
```

which will point to another table, where we can distinguish between opcodes such as:

```cpp
table8[0x0] = &Chip8::OP_8xy0; and table8[0x1] = &Chip8::OP_8xy1;
```

. Finally, the dispatch table is called when we decode the opcode in the main cycle as such:

```cpp
((*this).*(table[(opcode & 0xF000u) >> 12u]))();
```

which gets the first digit of the opcode. Then the function dispatch table is called via ```table```.


# Next steps

Get the sound to work!! I intentionally delayed this because this would require me to leanr SDL2 more in depth, for which I am currently short on time to do so.
