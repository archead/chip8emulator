#pragma once
#include <cstdint>
#include <string>
class Chip8
{
public:
	Chip8();
	void LoadROM(const std::string& filename);
	void Cycle();
private:
	uint8_t memory[4096]{};
	uint8_t V[16]{};
	uint16_t I = 0;
	uint16_t pc = 0x200;
	uint16_t stack[16]{};
	uint8_t sp = 0;
	uint8_t delayTimer = 0;
	uint8_t soundTimer = 0;
	uint8_t keypad[16]{};
	uint8_t display[64 * 32]{};

	void ExecuteOpcode(uint16_t opcode);
};

