// TODO: Statically Link Executable

#include <windows.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "shared.h"

//
// Assets
//
#include "assets/background_img.h"
#include "assets/paddle_img.h"
#include "assets/ball_img.h"
#include "assets/8bit_font.h"
#include "assets/sound_beep_wav.h"
#include "assets/sound_peep_wav.h"
#include "assets/sound_plop_wav.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define PADDLE_WIDTH 15
#define PADDLE_HEIGHT 100
#define BALL_RADIUS 10

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

struct paddle
{
    SDL_Texture *Texture = NULL;
    SDL_Rect Rect = {};
};

struct ball
{
    SDL_Texture *Texture = NULL;
    SDL_Rect Rect = {};
    i32 Radius;
};

struct gamestate
{
    state CurrentState;

    ball Ball = {};
    paddle PlayerOne = {};
    paddle PlayerTwo = {};

    // Game Textures
    SDL_Texture *TextureBackground = NULL;
    SDL_Texture *TextureBall = NULL;
    SDL_Texture *TexturePaddle = NULL;

    // Font
    TTF_Font *Font = NULL;
    SDL_Texture *GameOver = NULL;

    // Sounds
    Mix_Chunk *SoundBeep = NULL;
    Mix_Chunk *SoundPeep = NULL;
    Mix_Chunk *SoundPlop = NULL;

    // Time Related
    r64 DeltaTime;
    u64 FrameStart;
    u64 FrameEnd;
    u64 CounterFreq;
};

global_variable i32 IsRunning = true;
global_variable gamestate Gamestate = {};
global_variable keyboard Keyboard = {};
global_variable mouse Mouse = {};
global_variable SDL_Window *Window;
global_variable SDL_Renderer *Renderer;

SDL_Texture *MySDL_LoadImageFromArray(unsigned char *Data, i32 Size)
{
    SDL_Texture *Result = NULL;
    SDL_Surface *Surface = NULL;

    Surface = IMG_Load_RW(SDL_RWFromMem((void*)Data, Size), 1);
    Assert(Surface);

    Result = SDL_CreateTextureFromSurface(Renderer, Surface);
    Assert(Result);

    return (Result);
}

int LoadAssets(void)
{
    // Load Pong Textures
    Gamestate.TextureBackground = MySDL_LoadImageFromArray(_BackgroundBmp, _SizeBackgroundBmp);
    Gamestate.TextureBall       = MySDL_LoadImageFromArray(_BallBmp, _SizeBallBmp);
    Gamestate.TexturePaddle     = MySDL_LoadImageFromArray(_PaddleBmp, _SizePaddleBmp);

    // Sounds
    Gamestate.SoundBeep = Mix_LoadWAV_RW(SDL_RWFromMem((void*)_beep_wav, _Size_beep_wav), 1);
    Assert(Gamestate.SoundBeep);
    Gamestate.SoundPeep = Mix_LoadWAV_RW(SDL_RWFromMem((void*)_peep_wav, _Size_peep_wav), 1);
    Assert(Gamestate.SoundPeep);
    Gamestate.SoundPlop = Mix_LoadWAV_RW(SDL_RWFromMem((void*)_plop_wav, _Size_plop_wav), 1);
    Assert(Gamestate.SoundPlop);

    // Load Font
    Gamestate.Font = TTF_OpenFontRW(SDL_RWFromMem((void*)_Font, _SizeFont), 1, 39);
    Assert(Gamestate.Font);

    // Generate Main Menu text textures

    // TODO: Remove all these magic numbers
    // Create PlayerOne
    Gamestate.PlayerOne.Texture = Gamestate.TexturePaddle;
    Gamestate.PlayerOne.Rect = { 5, WINDOW_HEIGHT / 2 - PADDLE_HEIGHT / 2,
                                 PADDLE_WIDTH, PADDLE_HEIGHT};

    // Create PlayerTwo
    Gamestate.PlayerTwo.Texture = Gamestate.TexturePaddle;
    Gamestate.PlayerTwo.Rect = { WINDOW_WIDTH - 5 - PADDLE_WIDTH,
                                 WINDOW_HEIGHT / 2 - PADDLE_HEIGHT / 2,
                                 PADDLE_WIDTH, PADDLE_HEIGHT};

    // Create Ball
    Gamestate.Ball.Texture = Gamestate.TextureBall;
    Gamestate.Ball.Radius = BALL_RADIUS;
    Gamestate.Ball.Rect = {WINDOW_WIDTH / 2 - BALL_RADIUS,
                           WINDOW_HEIGHT / 2 - BALL_RADIUS,
                           BALL_RADIUS * 2, BALL_RADIUS * 2};

    return true;
}

i32 main(i32 argc, char **argv)
{
    // TODO: Check for error
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    Mix_Init(MIX_INIT_OGG);

    // TODO: Check for error
    Window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT, NULL);

    // TODO: Check for error
    Renderer = SDL_CreateRenderer(Window, -1,
                                  SDL_RENDERER_ACCELERATED |
                                  SDL_RENDERER_PRESENTVSYNC);

    //Initialize SDL_mixer
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
    }
    // Set volume to 1/8 max volume
    Mix_Volume(-1, MIX_MAX_VOLUME / 8);

    LoadAssets();

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

                if(Keyboard.State[SDL_SCANCODE_RETURN])
                {
                    Mix_PlayChannel( -1, Gamestate.SoundBeep, 0 );
                }


                if(Keyboard.State[SDL_SCANCODE_W])
                {
                    Gamestate.PlayerOne.Rect.y -= 2;
                }

                if(Keyboard.State[SDL_SCANCODE_S])
                {
                    Gamestate.PlayerOne.Rect.y += 2;
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

        //
        // Render
        //
        switch(Gamestate.CurrentState)
        {
            case State_Game:
            {

                SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
                SDL_SetTextureColorMod(Gamestate.TextureBackground, 255, 255, 255);
                SDL_SetTextureColorMod(Gamestate.PlayerOne.Texture, 255, 255, 255);
                SDL_SetTextureColorMod(Gamestate.PlayerTwo.Texture, 255, 255, 255);
                SDL_SetTextureColorMod(Gamestate.Ball.Texture, 255, 255, 255);

                SDL_RenderClear(Renderer);

                SDL_RenderCopy(Renderer, Gamestate.TextureBackground, NULL, NULL);
                SDL_RenderCopy(Renderer, Gamestate.PlayerOne.Texture, NULL, &Gamestate.PlayerOne.Rect);
                SDL_RenderCopy(Renderer, Gamestate.PlayerTwo.Texture, NULL, &Gamestate.PlayerTwo.Rect);
                SDL_RenderCopy(Renderer, Gamestate.Ball.Texture, NULL, &Gamestate.Ball.Rect);

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

                // Draw Tinted Background Game_State
                SDL_SetTextureColorMod(Gamestate.TextureBackground, 100, 100, 100);
                SDL_RenderCopy(Renderer, Gamestate.TextureBackground, 0, 0);

                SDL_SetTextureColorMod(Gamestate.PlayerOne.Texture, 100, 100, 100);
                SDL_RenderCopy(Renderer, Gamestate.PlayerOne.Texture, NULL, &Gamestate.PlayerOne.Rect);

                SDL_SetTextureColorMod(Gamestate.PlayerTwo.Texture, 100, 100, 100);
                SDL_RenderCopy(Renderer, Gamestate.PlayerTwo.Texture, NULL, &Gamestate.PlayerTwo.Rect);

                SDL_SetTextureColorMod(Gamestate.Ball.Texture, 100, 100, 100);
                SDL_RenderCopy(Renderer, Gamestate.Ball.Texture, NULL, &Gamestate.Ball.Rect);

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
