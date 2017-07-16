// TODO: Statically Link Executable
#pragma warning(disable: 4189)
#pragma warning(disable: 4201)
#pragma warning(disable: 4100)

#include <windows.h>
#include <SDL.h>
#include <SDL_image.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define Assert(Expr) assert(Expr)

#ifdef _WIN32
#define Free(Expr) VirtualFree((Expr), 0, MEM_RELEASE)
#define Malloc(Size) VirtualAlloc(NULL, (SIZE_T)(Size), MEM_COMMIT, PAGE_READWRITE)
#define ZeroMem(Ptr, Size) ZeroMemory(Ptr, Size)
#define ZeroArray(Ptr, Size) ZeroMemory(Ptr, Size)
#endif

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    r32;
typedef double   r64;
typedef i32      b32;

//
// Image Assets
//
#include "background_img.h"
#include "paddle_img.h"
#include "ball_img.h"

enum state
{
    State_Game,
    State_Menu,
    State_Pause,
    State_End
};

struct keyboard
{
    const u8 *State;
    u8 *PrevState;
    i32 Numkeys;
};

struct mouse
{
    i32 x;
    i32 y;
    u32 State;
    u32 PrevState;
};

struct gamestate
{
    state CurrentState;

    // Time Related
    r64 DeltaTime;
    u64 FrameStart;
    u64 FrameEnd;
    u64 CounterFreq;
};

static i32 IsRunning = true;
static i32 WindowWidth = 640;
static i32 WindowHeight = 480;
static gamestate Gamestate = {};
static keyboard Keyboard = {};
static mouse Mouse = {};

SDL_Texture *MySDL_LoadImageFromArray(SDL_Renderer *Renderer, unsigned char *Data, i32 Size)
{
    SDL_Texture *Result = NULL;
    SDL_Surface *Surface = NULL;

    Surface = IMG_Load_RW(SDL_RWFromMem((void*)Data, Size), 1);
    Assert(Surface);

    Result = SDL_CreateTextureFromSurface(Renderer, Surface);
    Assert(Result);

    return (Result);
}

i32 main(i32 argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *Window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WindowWidth, WindowHeight, NULL);
    SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1,
                                                SDL_RENDERER_ACCELERATED |
                                                SDL_RENDERER_PRESENTVSYNC);

    // Load Textures
    SDL_Texture *Background = MySDL_LoadImageFromArray(Renderer, _BackgroundBmp, _SizeBackgroundBmp);
    SDL_Texture *Ball = MySDL_LoadImageFromArray(Renderer, _BallBmp, _SizeBallBmp);
    SDL_Texture *Paddle = MySDL_LoadImageFromArray(Renderer, _PaddleBmp, _SizePaddleBmp);

    // Keyboard Setup
    Keyboard.State = SDL_GetKeyboardState(&Keyboard.Numkeys);
    Keyboard.PrevState = (u8*)malloc(sizeof(u8) * Keyboard.Numkeys);
    ZeroMem(Keyboard.PrevState, sizeof(u8) * Keyboard.Numkeys);

    // Timer Setup
    Gamestate.CounterFreq = SDL_GetPerformanceFrequency();

    while(IsRunning)
    {
        Gamestate.FrameStart = SDL_GetPerformanceCounter();

        // Copy Previous Keyboard State
        memcpy((void*)Keyboard.PrevState, (void*)Keyboard.State, sizeof(u8) * Keyboard.Numkeys);

        SDL_Event Event;
        while(SDL_PollEvent(&Event))
        {
            if(Event.type == SDL_QUIT)
            {
                IsRunning = false;
            }
        }

        Keyboard.State = SDL_GetKeyboardState(NULL);
        Mouse.PrevState = Mouse.State;
        Mouse.State = SDL_GetMouseState(&Mouse.x, &Mouse.y);

        //
        // Update
        //

        switch(Gamestate.CurrentState)
        {
            case State_Game:
            {
                if(Keyboard.State[SDL_SCANCODE_ESCAPE])
                {
                    Gamestate.CurrentState = State_Pause;
                }
                break;
            }
            case State_Menu:
            {
                printf("We are in Menu State, it is empty!\n");
                break;
            }
            case State_Pause:
            {
                // Escape exits game
                if(Keyboard.State[SDL_SCANCODE_ESCAPE] && !Keyboard.PrevState[SDL_SCANCODE_ESCAPE])
                {
                    IsRunning = false;
                }

                // Enter switches back to game
                if(Keyboard.State[SDL_SCANCODE_RETURN])
                {
                    Gamestate.CurrentState = State_Game;
                }
                break;
            }
            case State_End:
            {
                printf("We are in End State, it is empty!\n");
                break;
            }
        }

        SDL_Rect Rect = {0, 0, 100, 100};

        //
        // Render
        //

        switch(Gamestate.CurrentState)
        {
            case State_Game:
            {

                SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
                SDL_RenderClear(Renderer);

                SDL_RenderCopy(Renderer, Background, NULL, NULL);

                SDL_RenderPresent(Renderer);

                break;
            }
            case State_Menu:
            {
                break;
            }
            case State_Pause:
            {
                SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
                SDL_RenderClear(Renderer);

                SDL_SetTextureColorMod(Background, 100, 100, 100);
                SDL_RenderCopy(Renderer, Background, 0, 0);

                // SDL_RenderCopy(Renderer, Paddle, 0, &MyRect);
                // SDL_RenderCopy(Renderer, Ball, 0, 0);
                SDL_RenderPresent(Renderer);

                break;
            }
            case State_End:
            {
                break;
            }
        }


        Gamestate.FrameEnd = SDL_GetPerformanceCounter();
        Gamestate.DeltaTime = (r64)((Gamestate.FrameEnd - Gamestate.FrameStart) * 1000.0f) / Gamestate.CounterFreq;
        Gamestate.DeltaTime /= 1000.0f;
    }

    return 0;
}
