#include "Chip8.h"

#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>
#include <random>
#include <ctime>

Chip8::Chip8()
{
	// normally you would load the font here, TODO
	srand(time(0)); // for RND
	pc = 0x200; // set the program counter back to initial position in memory

	uint8_t hexSpritesBuffer[80] = {
		// 0
		0xF0, 0x90, 0x90, 0x90, 0xF0,
		// 1
		0x20, 0x60, 0x20, 0x20, 0x70,
		// 2
		0xF0, 0x10, 0xF0, 0x80, 0xF0,
		// 3
		0xF0, 0x10, 0xF0, 0x10, 0xF0,
		// 4
		0x90, 0x90, 0xF0, 0x10, 0x10,
		// 5
		0xF0, 0x80, 0xF0, 0x10, 0xF0,
		// 6
		0xF0, 0x80, 0xF0, 0x90, 0xF0,
		// 7
		0xF0, 0x10, 0x20, 0x40, 0x40,
		// 8
		0xF0, 0x90, 0xF0, 0x90, 0xF0,
		// 9
		0xF0, 0x90, 0xF0, 0x10, 0xF0,
		// A
		0xF0, 0x90, 0xF0, 0x90, 0x90,
		// B
		0xE0, 0x90, 0xE0, 0x90, 0xE0,
		// C
		0xF0, 0x80, 0x80, 0x80, 0xF0,
		// D
		0xE0, 0x90, 0x90, 0x90, 0xE0,
		// E
		0xF0, 0x80, 0xF0, 0x80, 0xF0,
		// F
		0xF0, 0x80, 0xF0, 0x80, 0x80
	};

	// load hex sprites into memory during init
	memcpy(&memory[0x50], hexSpritesBuffer, sizeof(hexSpritesBuffer));

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
	// increment the program counter by 2
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

	/*1nnn - JP addr
	Jump to location nnn.
	The interpreter sets the program counter to nnn.*/
	case 0x1000: {
		uint16_t NNN = opcode & 0x0FFF;
		pc = NNN;
		std::cout << "[1NNN] Jump to " << std::hex << NNN << std::dec << "\n";
		break;
	}
	
	/*2nnn - CALL addr
	Call subroutine at nnn.
	The interpreter increments the stack pointer, 
	then puts the current PC on the top of the stack.
	The PC is then set to nnn.*/
	case 0x2000: {
		uint16_t NNN = opcode & 0x0FFF;
		if (sp >= 15) {
			std::cerr << "[2NNN] ERROR: STACK OVERFLOW!";
			break;
		}
		//sp++;
		stack[sp] = pc;
		++sp;
		pc = NNN;
		break;
	}

	/*3xkk - SE Vx, byte
	Skip next instruction if Vx = kk.
	The interpreter compares register Vx to kk,
	and if they are equal, increments the program counter by 2*/
	case 0x3000: {
		uint8_t X = (opcode & 0x0F00) >> 8;
		uint8_t KK = (opcode & 0x0FF);

		if (V[X] == KK) {
			pc += 2;
			std::cout << "[3XKK] V[" << X << "] == " << KK << " - skipping instruction" << "\n";
			break;
		}
		
		std::cout << "[3XKK] V[" << X << "] != " << KK << " - continuing" << "\n";
		break;
	}

	/*4xkk - SNE Vx, byte
	Skip next instruction if Vx != kk.
	The interpreter compares register Vx to kk,
	and if they are not equal, increments the program counter by 2.*/
	case 0x4000: {
		uint8_t X = (opcode & 0x0F00) >> 8;
		uint8_t KK = opcode & 0x00FF;

		if (V[X] != KK) {
			pc += 2;
			std::cout << "[4XKK] V[" << X << "] != " << KK << " - skipping instruction" << "\n";
			break;
		}

		std::cout << "[4XKK] V[" << X << "] == " << KK << " - continuing" << "\n";
		break;
	}

	/*5xy0 - SE Vx, Vy
	Skip next instruction if Vx = Vy.
	The interpreter compares register Vx to register Vy,
	and if they are equal, increments the program counter by 2.*/
	case 0x5000: {
		uint8_t X = (opcode & 0x0F00) >> 8;
		uint8_t Y = (opcode & 0x00F0) >> 4;

		if (V[X] == V[Y]) {
			pc += 2;
			std::cout << "[5XY0] V[" << (int)X << "] == V[" << (int)Y << "] - skipping instruction" << "\n";
			break;
		}

		std::cout << "[5XY0] V[" << (int)X << "] != V[" << (int)Y << "] - continuing" << "\n";
		break;
	}

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
	
	case 0x8000: 
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
				std::cout << "8xy4 V[" << (int)X << "] = " << "V[" << (int)X << "] + " << "V[" << (int)Y << "]" << ", V[F] = 0x1 (Carry)" << "\n";
			}
			else {
				V[X] = SUM & 0x00FF; // store the lowest 8 bits as well since registers are 8-bit
				V[0xF] = 0x0;
				std::cout << "8xy4 V[" << (int)X << "] = " << "V[" << (int)X << "] + " << "V[" << (int)Y << "]" << ", V[F] = 0x0 (Carry)" << "\n";
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
			std::cout << "8xy5: V[" << (int)X << "] = " << "V[" << (int)X << "] - " << "V[" << (int)Y << "]\n";
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

		default:
			std::cerr << "Unknown 0x8 opcode: " << std::hex << opcode << "\n";
			break;
	}
		break;

	/*9xy0 - SNE Vx, Vy
	Skip next instruction if Vx != Vy.
	The values of Vx and Vy are compared, 
	and if they are not equal, the program counter is increased by 2.*/
	case 0x9000: {
		uint8_t X = (opcode & 0x0F00) >> 8;
		uint8_t Y = (opcode & 0x00F0) >> 4;

		if (V[X] != V[Y]) {
			pc += 2;
			std::cout << "[9XY0] V[" << (int)X << "] != V[" << (int)Y << "] - skipping instruction" << "\n";
			break;
		}

		std::cout << "[9XY0] V[" << (int)X << "] == V[" << (int)Y << "] - continuing" << "\n";
		break;
	}

	/*Annn - LD I, addr
	Set I = nnn.
	The value of register I is set to nnn.*/
	case 0xA000: {
		uint16_t NNN = opcode & 0x0FFF;
		I = NNN;

		std::cout << "[ANNN] I = " << (int)NNN << "\n";
		break;
	}

	/*Bnnn - JP V0, addr
	Jump to location nnn + V0.
	The program counter is set to nnn plus the value of V0.*/
	case 0xB000: {
		uint16_t NNN = opcode & 0x0FFF;
		pc = V[0x0] + NNN;
		std::cout << "[BNNN] Jump to " << std::hex << (int)pc << std::dec << "\n";
		break;
	}

	/*Cxkk - RND Vx, byte
	Set Vx = random byte AND kk.
	The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk.
	The results are stored in Vx. See instruction 8xy2 for more information on AND.*/
	case 0xC000: {
		uint8_t RND = rand() % 256;
		uint8_t X = (opcode & 0x0F00) >> 8;
		uint16_t KK = opcode & 0x00FF;
		V[X] = RND & KK;
		std::cout << "[CXKK] V[" << (int)X << "] = RND(" << RND << ") AND " << KK << "\n";
		break;
	}

	/*Dxyn - DRW Vx, Vy, nibble
	Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
	The interpreter reads n bytes from memory, starting at the address stored in I.
	These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
	Sprites are XORed onto the existing screen.
	If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
	If the sprite is positioned so part of it is outside the coordinates of the display, it wraps around to the opposite side of the screen.
	See instruction 8xy3 for more information on XOR, and section 2.4, Display, for more information on the Chip-8 screen and sprites.*/
	case 0xD000: {
		uint8_t X = (opcode & 0x0F00) >> 8;
		uint8_t Y = (opcode & 0x00F0) >> 4;
		uint8_t N = opcode & 0x000F;

		uint16_t xPos = V[X] % 64;
		uint16_t yPos = V[Y] % 32;

		bool collision = false;
		V[0xF] = 0; // reset collision

		for (int row = 0; row < N; row++) {

			uint16_t y = ((yPos + row) % 32) * 64;

			for (int col = 0; col < 8; col++) {

				uint8_t x = (xPos + col) % 64;

				uint8_t spriteStripe = memory[I + row];
				uint8_t bit = (spriteStripe >> (7 - col)) & 0x1;

				if (display[x + y] != 0 && bit != 0) collision = true;

				display[x + y] ^= bit;
			}
		}
		drawFlag = true; // wait for the entire buffer to be filled before drawing to screen
		if (collision) V[0xF] = 1;
		break;
	}

	case 0xE000:
		switch (opcode & 0x00FF) {

			/*Ex9E - SKP Vx
			Skip next instruction if key with the value of Vx is pressed.
			Checks the keyboard, and if the key corresponding to the value of Vx
			is currently in the down position, PC is increased by 2.*/
			case 0x009E: {
				uint8_t X = (opcode & 0x0F00) >> 8;
				if (keypad[V[X]]) {
					pc += 2;
					std::cout << "[EX9E] Key " << (int)V[X] << " is pressed - skipping instruction" << "\n";
				}
				break;
			}
			/*ExA1 - SKNP Vx
			Skip next instruction if key with the value of Vx is not pressed.
			Checks the keyboard, and if the key corresponding to the value of Vx
			is currently in the up position, PC is increased by 2.*/
			case 0x00A1: {
				uint8_t X = (opcode & 0x0F00) >> 8;
				if (!keypad[V[X]]) {
					pc += 2;
					std::cout << "[EXA1] Key " << (int)V[X] << " is not pressed - skipping instruction" << "\n";
				}
				break;
			}

			default:
				std::cerr << "Unknown 0xF opcode: " << std::hex << opcode << "\n";
				break;
		}
		break;
	case 0xF000:
		switch (opcode & 0x00FF) {

		/*Fx07 - LD Vx, DT
		Set Vx = delay timer value.
		The value of DT is placed into Vx.*/
		case 0x0007: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			V[X] = delayTimer;
			break;
		}
		
		/*Fx0A - LD Vx, K
		Wait for a key press, store the value of the key in Vx.
		All execution stops until a key is pressed, then the value of that key is stored in Vx.*/
		case 0x000A: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			bool pressed = false;
			while (!pressed) {
				for (int i = 0; i < 16; i++) {
					if (keypad[i]) {
						V[X] = i;
						pressed = true;
						break;
					}
				}
			}
			break;
		}

		/*Fx15 - LD DT, Vx
		Set delay timer = Vx.
		DT is set equal to the value of Vx.*/
		case 0x0015: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			delayTimer = V[X];
			break;
		}

		/*Fx18 - LD ST, Vx
		Set sound timer = Vx.
		ST is set equal to the value of Vx.*/
		case 0x0018: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			soundTimer = V[X];
			break;
		}

		/*Fx1E - ADD I, Vx
		Set I = I + Vx.
		The values of I and Vx are added, and the results are stored in I.*/
		case 0x001E: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			I += V[X];

			break;
		}

		/*Fx29 - LD F, Vx
		Set I = location of sprite for digit Vx.
		The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
		See section 2.4, Display, for more information on the Chip-8 hexadecimal font.
		The data should be stored in the interpreter area of Chip-8 memory (0x000 to 0x1FF).*/
		case 0x0029: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			I = 0x50 + (V[X] * 5);
			std::cout << "[FX29] I = " << (int)V[X] * 5 << "\n";
			break;
		}
		/*Fx33 - LD B, Vx
		Store BCD representation of Vx in memory locations I, I+1, and I+2.
		The interpreter takes the decimal value of Vx, 
		and places the hundreds digit in memory at location in I,
		the tens digit at location I+1, and the ones digit at location I+2.*/
		case 0x0033: {
			uint8_t X = (opcode & 0x0F00) >> 8;
			uint8_t Vx = V[X];

			memory[I] = Vx / 100;
			memory[I + 1] = (Vx / 10) % 10;
			memory[I + 2] = Vx % 10;

			std::cout << "[FX33] I = " << (int)Vx << "\n";
			break;
		}

		/*Fx55 - LD [I], Vx
		Store registers V0 through Vx in memory starting at location I.
		The interpreter copies the values of registers V0 through Vx into memory,
		starting at the address in I.*/
		case 0x0055: {
			uint8_t X = (opcode & 0x0F00) >> 8;

			// X + 1 since it's size of array not index
			memcpy(&memory[I], V, (X + 1) * sizeof(uint8_t));

			std::cout << "[FX55] Dump register from V[0] to V[" << int(X) << "]\n";
			break;
		}

		/*Fx65 - LD Vx, [I]
		Read registers V0 through Vx from memory starting at location I.
		The interpreter reads values from memory starting at location I
		into registers V0 through Vx.*/
		case 0x0065: {
			uint8_t X = (opcode & 0x0F00) >> 8;

			memcpy(V, &memory[I], (X + 1) * sizeof(uint8_t));

			std::cout << "[FX65] Load from I to registers from V[0] to V[" << int(X) << "]\n";
			break;
		}
		}
		break;
	
	default:
		std::cerr << "Unknown opcode: " << std::hex << opcode << "\n";
		break;
	}
}
