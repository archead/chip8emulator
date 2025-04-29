#include "Chip8.h"

#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>

Chip8::Chip8()
{
	// normally you would load the font here, TODO

	pc = 0x200; // set the program counter back to initial position in memory
}

void Chip8::LoadROM(const std::string& filename)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file) {
		std::cerr << "Failed to open ROM: " << filename << "\n";
		return;
	}

	// copy the input file stream into a vector as a buffer
	std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	//TODO: check that ROM fits into memory

	//copy the memory location of the buffer to the memory location of "memory" with the 0x200 offset as the start of the program
	std::copy(buffer.begin(), buffer.end(), memory + 0x200);
}

void Chip8::Cycle()
{
	// grabs first byte in memory array with program counter variable
	// bit shifts it 8 bits to the left since chip8 instructions are 16-bit
	// grab second byte in memory array, the value for the instruction
	// uses "or" | operator to join them together like so:
	// XXXX0000 + 0000XXXX
	uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
	// crement the program counter by 2
	// since each instruction is made out of 2 bytes
	pc += 2;
	// self explanatory lol
	ExecuteOpcode(opcode);

	//Decrement timers
	if (delayTimer > 0) --delayTimer;
	if (soundTimer > 0) --soundTimer;
}

void Chip8::ExecuteOpcode(uint16_t opcode)
{
	switch (opcode & 0xF000) {

	case 0x0000:
		switch (opcode & 0x00ff) {
		case 0x00E0:
			std::memset(display, 0, sizeof(display));
			std::cout << "Screen cleared\n";
			break;
		case 0x00EE:
			--sp;
			pc = stack[sp];
			std::cout << "Returning from subroutine\n";
			break;
		default:
			std::cerr << "Unknown 0x0 opcode: " << std::hex << opcode << "\n";
			break;
		}
		break;

	/*6xkk - LD Vx, byte
	Set Vx = kk.

	The interpreter puts the value kk into register Vx.*/

	case 0x6000: {
		uint8_t X = (opcode & 0x0F00) >> 8;
		uint8_t NN = opcode & 0x00FF;
		V[X] = NN;
		std::cout << "Set V[" << (int)X << "] = " << (int)NN << "\n";
		break;
	}

	/*7xkk - ADD Vx, byte
	Set Vx = Vx + kk.
	Adds the value kk to the value of register Vx, then stores the result in Vx.*/

	case 0x7000: { 
		uint8_t X = (opcode & 0x0F00) >> 8;
		uint8_t NN = opcode & 0x00FF;
		V[X] += NN;
		std::cout << "Add " << (int)NN << " to V[" << (int)X << "]\n";
		break;
	}
	
	case 0x8000: {
		switch (opcode & 0x000F) {

		/*8xy0 - LD Vx, Vy
		Set Vx = Vy.
		Stores the value of register Vy in register Vx.*/
		case 0x0000: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			uint8_t Y = (opcode & 0x00F0) >> 4;
			V[X] = V[Y];
			std::cout << "Set V[" << (int)X << "] = " << "V[" << (int)Y << "]\n";
			break;
		}
		
		/*8xy1 - OR Vx, Vy
		Set Vx = Vx OR Vy.
		Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx. 
		A bitwise OR compares the corrseponding bits from two values, and if either bit is 1, then the same bit in the result is also 1. 
		Otherwise, it is 0.*/
		case 0x0001: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			uint8_t Y = (opcode & 0x00F0) >> 4;
			V[X] = (V[X] | V[Y]);
			std::cout << "V[" << (int)X << "] = " << "V[" << (int)X << "] OR " << "V[" << (int)Y << "]\n";
			break;
		}

		/*8xy2 - AND Vx, Vy
		Set Vx = Vx AND Vy.
		Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
		A bitwise AND compares the corrseponding bits from two values,and if both bits are 1, then the same bit in the result is also 1.
		Otherwise, it is 0. */
		case 0x0002: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			uint8_t Y = (opcode & 0x00F0) >> 4;
			V[X] = (V[X] & V[Y]);
			std::cout << "V[" << (int)X << "] = " << "V[" << (int)X << "] AND " << "V[" << (int)Y << "]\n";
			break;
		}

		/*8xy3 - XOR Vx, Vy
		Set Vx = Vx XOR Vy.
		Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
		An exclusive OR compares the corrseponding bits from two values, and if the bits are not both the same, then the corresponding bit in the result is set to 1.
		Otherwise, it is 0. */
		case 0x0003: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			uint8_t Y = (opcode & 0x00F0) >> 4;
			V[X] = (V[X] ^ V[Y]);
			std::cout << "V[" << (int)X << "] = " << "V[" << (int)X << "] XOR " << "V[" << (int)Y << "]\n";
			break;
		}


		/* 8xy4 - ADD Vx, Vy
		 Set Vx = Vx + Vy, set VF = carry.
		The values of Vx and Vy are added together.
		If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0.
		Only the lowest 8 bits of the result are kept, and stored in Vx.*/
		case 0x0004: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			uint8_t Y = (opcode & 0x00F0) >> 4;
			uint16_t SUM = V[X] + V[Y];

			if (SUM > 255) {
				V[X] = SUM & 0x00FF; // store the lowest 8 bits (0xFF = 0b11111111)
				V[0xF] = 0x1;
				std::cout << "V[" << (int)X << "] = " << "V[" << (int)X << "] + " << "V[" << (int)Y << "]" << " V[F] = 0x1 (Carry)" << "\n";
			}
			else {
				V[X] = SUM & 0x00FF; // store the lowest 8 bits as well since registers are 8-bit
				V[0xF] = 0x0;
				std::cout << "V[" << (int)X << "] = " << "V[" << (int)X << "] + " << "V[" << (int)Y << "]" << " V[F] = 0x0 (Carry)" << "\n";
			}
			break;
		}
		/*8xy5 - SUB Vx, Vy
		 Set Vx = Vx - Vy, set VF = NOT borrow.
		 If Vx > Vy, then VF is set to 1, otherwise 0.
		 Then Vy is subtracted from Vx, and the results stored in Vx.*/
		case 0x0005: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			uint8_t Y = (opcode & 0x00F0) >> 4;
			if (V[X] > V[Y]) {
				V[0xF] = 1;
				V[X] -= V[Y];
			}
			else {
				V[0xF] = 0;
				V[X] -= V[Y];
			}
			std::cout << "V[" << (int)X << "] = " << "V[" << (int)X << "] - " << "V[" << (int)Y << "]\n";
			break;
		}
		/* 8xy6 - SHR Vx {, Vy}
		Set Vx = Vx SHR 1.
		If the least-significant bit of Vx is 1, 
		then VF is set to 1, otherwise 0. 
		Then Vx is divided by 2.*/
		case 0x0006: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			uint8_t Y = (opcode & 0x00F0) >> 4;
			
			if (V[X] & 0b00000001) {
				V[0xF] = 1;
			}
			else {
				V[0xF] = 0;
			}
			V[X] = V[X] >> 1;
			std::cout << "V[" << (int)X << "] = " << "V[" << (int)X << "] SHR " << "1\n";
			break;
		}

		/*8xy7 - SUBN Vx, Vy
		Set Vx = Vy - Vx, set VF = NOT borrow.
		If Vy > Vx, then VF is set to 1, otherwise 0. 
		Then Vx is subtracted from Vy, and the results stored in Vx.*/
		case 0x0007: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			uint8_t Y = (opcode & 0x00F0) >> 4;
			V[0xF] = V[Y] > V[X] ? 1 : 0;
			V[X] = V[Y] - V[X];
			std::cout << "V[" << (int)X << "] = " << "V[" << (int)Y << "] - [" << (int)X << "]\n";
			break;
		}

		/*8xyE - SHL Vx {, Vy}
		Set Vx = Vx SHL 1.
		If the most-significant bit of Vx is 1, 
		then VF is set to 1, otherwise to 0. 
		Then Vx is multiplied by 2.*/
		case 0x000E: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			uint8_t Y = (opcode & 0x00F0) >> 4;
			V[0xF] = (V[X] & 0b10000000) ? 1 : 0;
			V[X] = V[X] << 1;
			std::cout << "V[" << (int)X << "] = " << "V[" << (int)X << "] SHL " << "1\n";
			break;
		}
	}
}
	default:
		std::cerr << "Unknown opcode: " << std::hex << opcode << "\n";
		break;
	}
}
