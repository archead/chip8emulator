// chip8emulator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <SDL2/SDL.h>
#include "Chip8.h"
#include <iostream>
#include <chrono>
#include <thread>

const int CHIP8_WIDTH = 64;
const int CHIP8_HEIGHT = 32;

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 480;

const int PIXEL_WIDTH = SCREEN_WIDTH / CHIP8_WIDTH;
const int PIXEL_HEIGHT = SCREEN_HEIGHT / CHIP8_HEIGHT;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
bool init();
void close();
bool PointerCheck(void* SDL_Object);

int main(int argc, char* argv[])
{
    Chip8 chip8;
    chip8.LoadROM("particle_demo.ch8"); // load your own ROM here

    if (!init()) {
        printf("SDL INIT FAIL!");
    }
    else {
        bool quit = false;
        SDL_Event e;
        while (quit == false) {
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) quit = true;
            }
            chip8.Cycle();
            if (chip8.drawFlag) {
                SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xFF);
                SDL_RenderClear(gRenderer);
                for (int y = 0; y < 32; y++)
                {
                    for (int x = 0; x < 64; x++) {
                        if (chip8.display[x + y * 64]) {
                            SDL_Rect pixel_rect = {
                                x * PIXEL_WIDTH,
                                y * PIXEL_HEIGHT,
                                PIXEL_WIDTH,
                                PIXEL_HEIGHT
                            };
                            SDL_SetRenderDrawColor(gRenderer, 0xE0, 0xE0, 0xE0, 0x10);
                            SDL_RenderFillRect(gRenderer, &pixel_rect);
                        }
                    }
                }
                chip8.drawFlag = false;
            }

            SDL_RenderPresent(gRenderer);
            std::this_thread::sleep_for(std::chrono::milliseconds(2)); // Roughly 500Hz (the clock of CHIP8)
        }
    }

    return 0;
}

bool init()
{
    bool success = true;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("%s\n", SDL_GetError());
        success = false;
    }
    else {
        gWindow = SDL_CreateWindow("Chip8",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN);
        if (PointerCheck(gWindow)) success = false;
        else {
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
            if (PointerCheck(gRenderer)) success = false;
            else {
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            }
        }
    }
    return success;
}

void close()
{
    SDL_DestroyWindow(gWindow);
    SDL_DestroyRenderer(gRenderer);
    gWindow = NULL;
    gRenderer = NULL;

    SDL_Quit();
}

bool PointerCheck(void* SDL_Object)
{
    if (SDL_Object == NULL) {
        printf("ERROR: %s\n", SDL_GetError());
        return true;  
    }

    return false;
}
