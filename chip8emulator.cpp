// chip8emulator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Chip8.h"
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    Chip8 chip8;
    chip8.LoadROM("Pong.ch8");
    while (true) {
        chip8.Cycle();
        std::this_thread::sleep_for(std::chrono::milliseconds(2)); // Roughly 500Hz
    }

    return 0;
}