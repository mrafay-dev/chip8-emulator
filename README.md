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
