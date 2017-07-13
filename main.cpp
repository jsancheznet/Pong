// TODO: Statically Link Executable
#pragma warning(disable: 4189)
#pragma warning(disable: 4201)
#pragma warning(disable: 4100)

#pragma comment(lib, "Shcore.lib")

#include <windows.h>
#include <ShellScalingAPI.h>
#include <SDL.h>
#include "../assets/background.xpm"
#include <SDL_image.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

typedef char     u8;
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

enum state
{
    State_Game,
    State_Menu,
    State_Pause,
    State_End
};

// struct ball
// {
//     v2 Position;
//     r32 Radius;
//     r32 r, g, b, a;
// };

struct gamestate
{
    state CurrentState;
};


static i32 IsRunning = true;
static i32 WindowWidth = 640;
static i32 WindowHeight = 480;
static gamestate Gamestate = {};

i32 main(i32 argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *Window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WindowWidth, WindowHeight, NULL);
    SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1,
                                                SDL_RENDERER_ACCELERATED |
                                                SDL_RENDERER_PRESENTVSYNC);

    SDL_Surface *Image = IMG_ReadXPMFromArray(background_xpm);
    SDL_Texture *Texture = SDL_CreateTextureFromSurface(Renderer, Image);

    while(IsRunning)
    {
        SDL_Event Event;
        while(SDL_PollEvent(&Event))
        {
            switch(Event.type)
            {
                case SDL_KEYDOWN:
                {
                    break;
                }
                case SDL_KEYUP:
                {
                    if(Event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        IsRunning = false;
                    }
                    break;
                }
                case SDL_QUIT:
                {
                    IsRunning = false;
                    break;
                }
            }
        }

        SDL_Rect Rect = {0, 0, 100, 100};

        // Render
        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
        SDL_RenderClear(Renderer);

        SDL_RenderCopy(Renderer,
                       Texture, 0, 0);

        SDL_SetRenderDrawColor(Renderer, 255, 0, 255, 255);
        SDL_RenderDrawRect(Renderer, &Rect);
        SDL_RenderPresent(Renderer);
    }

    return 0;
}
