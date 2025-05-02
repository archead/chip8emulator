// chip8emulator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Chip8.h"
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    Chip8 chip8;
    chip8.LoadROM("1-chip8-logo.ch8"); // from https://github.com/Timendus/chip8-test-suite
    while (true) {
        for (int x = 0; x < 10; x++) chip8.Cycle(); // temporary workaround for improve rendering

        if (chip8.drawFlag)
        {
            std::cout << "\x1b[?25l" << "\x1b[H"; // clears the screen
            for (int y = 0; y < 32; y++)
            {
                for (int x = 0; x < 64; x++) {
                    char pixel = chip8.display[x + y * 64] ? 178 : 176;
                    std::cout << pixel;
                }
                std::cout << '\n';
            }
            chip8.drawFlag = false;
        }
        

        std::this_thread::sleep_for(std::chrono::milliseconds(2)); // Roughly 500Hz

    }

    return 0;
}