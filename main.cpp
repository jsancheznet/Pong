// TODO: Statically Link Executable

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <assert.h>

#include "pong_math.h"
#include "shared.h"

//
// Assets
//
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

struct ball
{
    v2 Velocity;
    v2 Acceleration;
    v2 Position;
    r32 Speed;
};

struct paddle
{
    v2 Position;
    v2 Velocity;
    v2 Acceleration;
    r32 Speed;
};

struct gamestate
{
    state CurrentState;

    SDL_Rect Background[3] = {};

    ball Ball = {};
    paddle PlayerOne = {};
    paddle PlayerTwo = {};

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
global_variable r64 DeltaTime = 0;
global_variable gamestate Gamestate = {};
global_variable keyboard Keyboard = {};
global_variable mouse Mouse = {};
global_variable SDL_Window *Window;
global_variable SDL_Renderer *Renderer;


void EntityUpdate(v2 *Position, v2 *Velocity, v2 *Acceleration, r64 DT)
{
    r32 dt = (r32)DT;
    *Position = 0.5f * (*Acceleration) * (dt * dt) + *Velocity + *Position;
    *Velocity = *Acceleration * dt + *Velocity;
    *Acceleration = {};
}

#if 0
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
#endif

int LoadAssets(void)
{
    // Load Sounds
    Gamestate.SoundBeep = Mix_LoadWAV_RW(SDL_RWFromMem((void*)_beep_wav, _Size_beep_wav), 1);
    Assert(Gamestate.SoundBeep);
    Gamestate.SoundPeep = Mix_LoadWAV_RW(SDL_RWFromMem((void*)_peep_wav, _Size_peep_wav), 1);
    Assert(Gamestate.SoundPeep);
    Gamestate.SoundPlop = Mix_LoadWAV_RW(SDL_RWFromMem((void*)_plop_wav, _Size_plop_wav), 1);
    Assert(Gamestate.SoundPlop);

    // Load Font
    Gamestate.Font = TTF_OpenFontRW(SDL_RWFromMem((void*)_Font, _SizeFont), 1, 39);
    Assert(Gamestate.Font);

    const i32 BgLineSpan = 10;
    // Background Top Line
    Gamestate.Background[0] = {0, 0, WINDOW_WIDTH, BgLineSpan};
    // Background Bottom Line
    Gamestate.Background[1] = {0, WINDOW_HEIGHT - BgLineSpan, WINDOW_WIDTH, BgLineSpan};
    // Background Mid Line
    Gamestate.Background[2] = {WINDOW_WIDTH / 2 - (BgLineSpan / 2), 0, BgLineSpan, WINDOW_HEIGHT};

    // Create PlayerOne
    const int PaddlePadding = 10;
    Gamestate.PlayerOne.Position = {(r32)PaddlePadding, (r32)(WINDOW_HEIGHT / 2 - PADDLE_HEIGHT / 2)};
    Gamestate.PlayerOne.Speed = 4.0f;

    // Create PlayerTwo
    Gamestate.PlayerTwo = { WINDOW_WIDTH - PaddlePadding - PADDLE_WIDTH,
                            WINDOW_HEIGHT / 2 - PADDLE_HEIGHT / 2,
                            PADDLE_WIDTH, PADDLE_HEIGHT};

    // Create Ball
    Gamestate.Ball.Position = {(r32)(WINDOW_WIDTH / 2 - BALL_RADIUS),
                               (r32)(WINDOW_HEIGHT / 2 - BALL_RADIUS)};

    return true;
}

enum direction
{
    UP,
    DOWN,
};
void PaddleMove(SDL_Rect *Rect, direction Direction)
{
    if(Direction == UP)
    {
        Rect->y -= 2;
    }
    else
    {
        Rect->y += 2;
    }

    // Wall Collision
    if(Rect->y <= 0)
    {
        Rect->y = 0;
    }
    if(Rect->y >= WINDOW_HEIGHT - PADDLE_HEIGHT)
    {
        Rect->y = WINDOW_HEIGHT - PADDLE_HEIGHT;
    }
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
    if(Window)
    {
        // TODO: Check for error
        Renderer = SDL_CreateRenderer(Window, -1,
                                      SDL_RENDERER_ACCELERATED |
                                      SDL_RENDERER_PRESENTVSYNC);
        if(!Renderer)
        {
            printf("Failed creating Renderer, aborting\n");
            return -2;
        }
    }
    else
    {
        printf("Failed Creating  Window, aborting\n");
        return -1;
    }

    //Initialize SDL_mixer
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
    }
    // Set volume to 1/8 max volume
    Mix_Volume(-1, MIX_MAX_VOLUME / 10);

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

                // Movement Player1
                if(Keyboard.State[SDL_SCANCODE_W])
                {
                    Gamestate.PlayerOne.Acceleration.y -= Gamestate.PlayerOne.Speed;
                }
                if(Keyboard.State[SDL_SCANCODE_S])
                {
                    Gamestate.PlayerOne.Acceleration.y += Gamestate.PlayerOne.Speed;
                }

                if(Keyboard.State[SDL_SCANCODE_SPACE])
                {
                    Gamestate.Ball.Acceleration += v2{5.0f, 5.0f};
                }

                // Ball Update
                EntityUpdate(&Gamestate.Ball.Position,
                             &Gamestate.Ball.Velocity,
                             &Gamestate.Ball.Acceleration,
                             DeltaTime);

                // Ball PlayerOne
                EntityUpdate(&Gamestate.PlayerOne.Position,
                             &Gamestate.PlayerOne.Velocity,
                             &Gamestate.PlayerOne.Acceleration,
                             DeltaTime);

                // Ball Collision
                if(Gamestate.Ball.Position.x <= 0)
                {
                    Gamestate.Ball.Velocity.x *= -1.0f;
                }
                if(Gamestate.Ball.Position.x >= (WINDOW_WIDTH - BALL_RADIUS * 2))
                {
                    Gamestate.Ball.Velocity.x *= -1.0f;
                }
                if(Gamestate.Ball.Position.y <= 0)
                {
                    Gamestate.Ball.Velocity.y *= -1.0f;
                }
                if(Gamestate.Ball.Position.y >= WINDOW_HEIGHT - BALL_RADIUS * 2)
                {
                    Gamestate.Ball.Velocity.y *= -1.0f;
                }

                // Player Collision
                if(Gamestate.PlayerOne.Position.y <= 0)
                {
                    Gamestate.PlayerOne.Position.y = 0;
                    Gamestate.PlayerOne.Velocity = {};
                }
                if(Gamestate.PlayerOne.Position.y >= WINDOW_HEIGHT - PADDLE_HEIGHT)
                {
                    Gamestate.PlayerOne.Position.y = WINDOW_HEIGHT - PADDLE_HEIGHT;
                    Gamestate.PlayerOne.Velocity = {};
                }

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
                SDL_RenderClear(Renderer);

                SDL_SetRenderDrawColor(Renderer, 10, 100, 30, 255);
                SDL_RenderFillRects(Renderer, Gamestate.Background, 3);

                SDL_SetRenderDrawColor(Renderer, 40, 140, 40, 255);

                // Create Rects, and Draw Players
                SDL_Rect P1R = {(i32)Gamestate.PlayerOne.Position.x,
                                (i32)Gamestate.PlayerOne.Position.y,
                                PADDLE_WIDTH,
                                PADDLE_HEIGHT};
                SDL_Rect P2R = {(i32)Gamestate.PlayerTwo.Position.x,
                                (i32)Gamestate.PlayerTwo.Position.y,
                                PADDLE_WIDTH,
                                PADDLE_HEIGHT};
                SDL_RenderFillRect(Renderer, &P1R);
                SDL_RenderFillRect(Renderer, &P2R);

                SDL_Rect BallRect = {(i32)Gamestate.Ball.Position.x,
                                     (i32)Gamestate.Ball.Position.y,
                                     BALL_RADIUS * 2,
                                     BALL_RADIUS * 2};

                SDL_RenderFillRect(Renderer, &BallRect);

                SDL_RenderPresent(Renderer);

                break;
            }
            case State_Pause:
            {
                SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
                SDL_RenderClear(Renderer);

                SDL_SetRenderDrawColor(Renderer, 0, 80, 10, 255);
                SDL_RenderFillRects(Renderer, Gamestate.Background, 3);

                SDL_SetRenderDrawColor(Renderer, 10, 100, 10, 255);

                SDL_Rect P1R = {(i32)Gamestate.PlayerOne.Position.x,
                                (i32)Gamestate.PlayerOne.Position.y,
                                PADDLE_WIDTH,
                                PADDLE_HEIGHT};

                SDL_Rect P2R = {(i32)Gamestate.PlayerTwo.Position.x,
                                (i32)Gamestate.PlayerTwo.Position.y,
                                PADDLE_WIDTH,
                                PADDLE_HEIGHT};

                SDL_RenderFillRect(Renderer, &P1R);
                SDL_RenderFillRect(Renderer, &P2R);


                SDL_Rect BallRect = {(i32)Gamestate.Ball.Position.x,
                                     (i32)Gamestate.Ball.Position.y,
                                     BALL_RADIUS*2,
                                     BALL_RADIUS*2};
                SDL_RenderFillRect(Renderer, &BallRect);

                SDL_RenderPresent(Renderer);

                break;
            }
            case State_End:
            {
                break;
            }
        }


        Gamestate.FrameEnd = SDL_GetPerformanceCounter();
        DeltaTime = (r64)((Gamestate.FrameEnd - Gamestate.FrameStart) * 1000.0f) / Gamestate.CounterFreq;
        DeltaTime /= 1000.0f;
    }

    return 0;
}
