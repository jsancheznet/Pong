// TODO: Statically Link Executable
// TODO: Remove position on paddles and balls and just use SDL_Rect?
// TODO: Create Ball Diameter Macro

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
#define PADDLE_HEIGHT 99
#define BALL_RADIUS 10
#define TEXT_COLOR {120, 160, 100}
#define SCORE_TEXT_COLOR {20, 110, 10}

enum state
{
    State_Begin,
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
    v2 Speed;
    SDL_Rect Rect;
};

struct paddle
{
    i32 Score;
    i32 Speed;
    SDL_Rect Rect;
};

struct gamestate
{
    state CurrentState;

    // Background made of SDL_Rects
    SDL_Rect Background[3] = {};

    // Font
    TTF_Font *FontBig = 0;
    TTF_Font *FontSmall = 0;
    SDL_Texture *GameOver = 0;
    SDL_Texture *Title = 0;
    SDL_Texture *PressSpaceToBegin = 0;
    SDL_Texture *MovementExplanation = 0;
    SDL_Texture *HumanWon = 0;
    SDL_Texture *HumanLost = 0;

    u64 LastCollision = 0;


    // Sounds
    Mix_Chunk *Beep = 0;
    Mix_Chunk *Peep = 0;
    Mix_Chunk *Plop = 0;

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
global_variable ball Ball = {};
global_variable paddle PlayerOne = {};
global_variable paddle PlayerTwo = {};

b32 Collision(paddle *Paddle, ball *PongBall)
{
    // TODO: Only do collision if 4-5 frames have passed, this will
    // stop the weird behaviour of the ball if the paddle is going the
    // direction the ball has bounced to.

    // // Update Rects
    // Paddle->Rect.x = (i32)Paddle->Position.x;
    // Paddle->Rect.y = (i32)Paddle->Position.y;
    // PongBall->Rect.x = (i32)PongBall->Position.x;
    // PongBall->Rect.y = (i32)PongBall->Position.y;

    // Check for Collision, if true
    // CollisionResult, check which one is smaller width or height.
    // Move the ball the smaller value back to its place.
    // Inverse the ball speed according
    SDL_Rect CollisionResult = {};
    if(SDL_IntersectRect(&Paddle->Rect, &PongBall->Rect, &CollisionResult) == SDL_TRUE)
    {
        Mix_PlayChannel( -1, Gamestate.Plop, 0 );

        // Handle X Collision
        if(CollisionResult.w < CollisionResult.h)
        {
            if(Paddle->Rect.x <= WINDOW_WIDTH / 2)
            {
                // Paddle is the left one
                PongBall->Rect.x += CollisionResult.w;
                PongBall->Speed.x *= -1.0f;
            }
            else
            {
                // Paddle is the right one
                PongBall->Rect.x -= CollisionResult.w;
                PongBall->Speed.x *= -1.0f;
            }

        }

        if(CollisionResult.h < CollisionResult.w)
        {
            // NOTE: if ball is above, substract, if below add
            if(PongBall->Rect.x < Paddle->Rect.x)
            {
                PongBall->Rect.y -= CollisionResult.h;
                PongBall->Speed.y *= -1.0f;
            }
            else
            {
                PongBall->Rect.y += CollisionResult.h;
                PongBall->Speed.y *= -1.0f;
            }
        }

        return true;
    }

    return false;
}

SDL_Texture *CreateTextureFromText(const char *String, TTF_Font *Font, SDL_Color Color)
{
    SDL_Texture *Result = 0;

    SDL_Color InputColor = {Color.r, Color.g, Color.b};
    SDL_Surface *Surface = 0;
    Surface = TTF_RenderText_Solid(Font, String, InputColor);
    if(!Surface)
    {
        printf("Error Creating TTF Surface: %s\n", TTF_GetError());
    }
    else
    {
        Result = SDL_CreateTextureFromSurface(Renderer, Surface);
        if(!Result)
        {
            printf("Error Creating the Texture from surface: %s\n", IMG_GetError());
            return NULL;
        }
    }

    SDL_FreeSurface(Surface);

    return Result;
}

int LoadAssets(void)
{
    // Load Sounds
    Gamestate.Beep = Mix_LoadWAV_RW(SDL_RWFromMem((void*)_beep_wav, _Size_beep_wav), 1); Assert(Gamestate.Beep);
    Gamestate.Peep = Mix_LoadWAV_RW(SDL_RWFromMem((void*)_peep_wav, _Size_peep_wav), 1); Assert(Gamestate.Peep);
    Gamestate.Plop = Mix_LoadWAV_RW(SDL_RWFromMem((void*)_plop_wav, _Size_plop_wav), 1); Assert(Gamestate.Plop);

    // Load Font
    Gamestate.FontBig = TTF_OpenFontRW(SDL_RWFromMem((void*)_Font, _SizeFont), 1, 39); Assert(Gamestate.FontBig);
    Gamestate.FontSmall = TTF_OpenFontRW(SDL_RWFromMem((void*)_Font, _SizeFont), 1, 18); Assert(Gamestate.FontSmall);

    // SDL_Texture *CreateTextureFromText(const char *String, TTF_Font *Font, u8 r, u8 g, u8 b)
    Gamestate.Title = CreateTextureFromText("Pong", Gamestate.FontBig, TEXT_COLOR);
    Gamestate.PressSpaceToBegin = CreateTextureFromText("Press space to begin", Gamestate.FontSmall, TEXT_COLOR);
    Gamestate.MovementExplanation = CreateTextureFromText("W  S  to control paddle", Gamestate.FontSmall, TEXT_COLOR);
    Gamestate.HumanWon = CreateTextureFromText("The Human has won", Gamestate.FontBig, TEXT_COLOR);
    Gamestate.HumanLost = CreateTextureFromText("HAL has won", Gamestate.FontBig, TEXT_COLOR);


    // Background comprised of SDL_Rects
    const i32 BgLineSpan = 10;
    Gamestate.Background[0] = {0, 0, WINDOW_WIDTH, BgLineSpan};                                    // Background Top Line
    Gamestate.Background[1] = {0, WINDOW_HEIGHT - BgLineSpan, WINDOW_WIDTH, BgLineSpan};           // Background Bottom Line
    Gamestate.Background[2] = {WINDOW_WIDTH / 2 - (BgLineSpan / 2), 0, BgLineSpan, WINDOW_HEIGHT}; // Background Mid Line

    // Create PlayerOne
    const i32 PlayerSpeed = 5;
    const int PaddlePadding = 10;
    PlayerOne.Rect = {PaddlePadding, (WINDOW_HEIGHT / 2 - PADDLE_HEIGHT / 2), PADDLE_WIDTH, PADDLE_HEIGHT};
    PlayerOne.Speed = PlayerSpeed;
    PlayerOne.Score = 0;

    // Create PlayerTwo
    PlayerTwo.Rect = {WINDOW_WIDTH - PaddlePadding - PADDLE_WIDTH, WINDOW_HEIGHT / 2 - PADDLE_HEIGHT / 2,
                      PADDLE_WIDTH, PADDLE_HEIGHT};
    PlayerTwo.Speed = PlayerSpeed;
    PlayerTwo.Score = 0;

    // Create Ball
    Ball.Rect = {(WINDOW_WIDTH / 2 - BALL_RADIUS), (WINDOW_HEIGHT / 2 - BALL_RADIUS),
                 BALL_RADIUS * 2, BALL_RADIUS * 2};

    return true;
}

void DrawTextureCentered(SDL_Texture *Texture, i32 x, i32 y)
{
    SDL_Rect DestRect = {};
    SDL_QueryTexture(Texture, NULL, NULL, &DestRect.w, &DestRect.h);
    DestRect.x = x - DestRect.w / 2;
    DestRect.y = y - DestRect.h / 2;

    SDL_RenderCopy(Renderer,
                   Texture,
                   NULL,
                   &DestRect);

}

void DrawScores(paddle *P1, paddle *P2)
{
    char ScoreP1[2] = {};
    char ScoreP2[2] = {};

    _itoa_s(P1->Score, ScoreP1, 2, 10);
    _itoa_s(P2->Score, ScoreP2, 2, 10);

    SDL_Texture *TextureP1 =  CreateTextureFromText(
        ScoreP1,
        Gamestate.FontBig, SCORE_TEXT_COLOR);
    SDL_Texture *TextureP2 =  CreateTextureFromText(
        ScoreP2,
        Gamestate.FontBig, SCORE_TEXT_COLOR);

    if(P1->Rect.x <= WINDOW_WIDTH / 2)
    {
        // Its on the left side
        // Draw P1 on left, p2 on right
        DrawTextureCentered(TextureP1, WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2);
        DrawTextureCentered(TextureP2, WINDOW_WIDTH / 2 + 100, WINDOW_HEIGHT / 2);
    }
    else
    {
        // Its on the right side
        // Draw p2 on left, p1 on right
        DrawTextureCentered(TextureP1, WINDOW_WIDTH / 2 + 100, WINDOW_HEIGHT / 2);
        DrawTextureCentered(TextureP2, WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2);
    }
}

void DrawBackgroundAndScores()
{
    SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
    SDL_RenderClear(Renderer);
    SDL_SetRenderDrawColor(Renderer, 10, 100, 30, 255);
    SDL_RenderFillRects(Renderer, Gamestate.Background, 3);
    DrawScores(&PlayerOne, &PlayerTwo);
}

void DrawPaddlesAndBall()
{

    // PlayerOne.Rect.x  = (i32)PlayerOne.Position.x;
    // PlayerOne.Rect.y  = (i32)PlayerOne.Position.y;
    // PlayerTwo.Rect.x  = (i32)PlayerTwo.Position.x;
    // PlayerTwo.Rect.y  = (i32)PlayerTwo.Position.y;
    // Ball.Rect.x  = (i32)Ball.Position.x;
    // Ball.Rect.y  = (i32)Ball.Position.y;

    SDL_RenderFillRect(Renderer, &PlayerOne.Rect);
    SDL_RenderFillRect(Renderer, &PlayerTwo.Rect);

    SDL_RenderFillRect(Renderer, &Ball.Rect);

}

i32 main(i32 argc, char **argv)
{
    // TODO: Check for error
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    Mix_Init(MIX_INIT_OGG);

    Window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT, NULL);
    if(Window)
    {
        Renderer = SDL_CreateRenderer(Window, -1,
                                      SDL_RENDERER_ACCELERATED |
                                      SDL_RENDERER_PRESENTVSYNC);
        if(!Renderer)
        {
            printf("Failed creating Renderer, aborting\n");
            return -2;
        }
        SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);
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

    // Set Currente State
    Gamestate.CurrentState = State_Begin;

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
            case State_Begin:
            {
                if(Keyboard.State[SDL_SCANCODE_SPACE])
                {
                    Gamestate.CurrentState = State_Game;
                }
                break;
            }
            case State_Game:
            {
                // Change state to State_Pause
                if(Keyboard.State[SDL_SCANCODE_ESCAPE])
                {
                    Gamestate.CurrentState = State_Pause;
                }

                // Movement PlayerOne
                if(Keyboard.State[SDL_SCANCODE_W])
                {
                    PlayerOne.Rect.y -= PlayerOne.Speed;
                }
                if(Keyboard.State[SDL_SCANCODE_S])
                {
                    PlayerOne.Rect.y += PlayerOne.Speed;
                }

                // AI Movement PlayerTwo
                if(Ball.Rect.y <= PlayerTwo.Rect.y - (PADDLE_HEIGHT) / 2)
                {
                    PlayerTwo.Rect.y -= PlayerTwo.Speed;
                }
                else if(Ball.Rect.y >= PlayerTwo.Rect.y + (PADDLE_HEIGHT / 2))
                {
                    PlayerTwo.Rect.y += PlayerTwo.Speed;
                }

                if(Keyboard.State[SDL_SCANCODE_SPACE])
                {
                    Ball.Speed += V2(-0.5f, 1.0f);
                }

                PlayerOne.Rect.x = (i32)PlayerOne.Rect.x;
                PlayerOne.Rect.y = (i32)PlayerOne.Rect.y;

                // Ball Update
                Ball.Rect.x += (i32)Ball.Speed.x;
                Ball.Rect.y += (i32)Ball.Speed.y;

                // Calculate collisions
                Collision(&PlayerOne, &Ball);
                Collision(&PlayerTwo, &Ball);

                // Ball Collision on Top and Bot walls
                if(Ball.Rect.y <= 0)
                {
                    Ball.Speed.y *= -1.0f;
                    Mix_PlayChannel( -1, Gamestate.Beep, 0 );
                }
                if(Ball.Rect.y >= WINDOW_HEIGHT - BALL_RADIUS * 2)
                {
                    Ball.Speed.y *= -1.0f;
                    Mix_PlayChannel( -1, Gamestate.Beep, 0 );
                }

                // Check if someone scored, put ball back into position
                if(Ball.Rect.x <= 0)
                {
                    Mix_PlayChannel( -1, Gamestate.Peep, 0 );
                    PlayerTwo.Score++;
                    Ball.Rect = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
                    if(PlayerTwo.Score >= 5)
                    {
                        Gamestate.CurrentState = State_End;
                    }
                }
                if(Ball.Rect.x >= (WINDOW_WIDTH - BALL_RADIUS * 2))
                {
                    Mix_PlayChannel( -1, Gamestate.Peep, 0 );
                    PlayerOne.Score++;
                    Ball.Rect = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
                    if(PlayerOne.Score >= 5)
                    {
                        Gamestate.CurrentState = State_End;
                    }
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
                // Space switches back to game
                if(Keyboard.State[SDL_SCANCODE_SPACE])
                {
                    Gamestate.CurrentState = State_Game;
                }
                break;
            }
            case State_End:
            {
                break;
            }
        }

        //
        // Render
        //

        switch(Gamestate.CurrentState)
        {
            case State_Begin:
            {
                // Draw Background and Scores
                DrawBackgroundAndScores();

                // Paddles & Ball
                SDL_SetRenderDrawColor(Renderer, 10, 100, 10, 255);
                SDL_RenderFillRect(Renderer, &PlayerOne.Rect);
                SDL_RenderFillRect(Renderer, &PlayerTwo.Rect);
                SDL_RenderFillRect(Renderer, &Ball.Rect);

                // Draw Text
                DrawTextureCentered(Gamestate.Title, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 200);
                DrawTextureCentered(Gamestate.MovementExplanation, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 100);
                DrawTextureCentered(Gamestate.PressSpaceToBegin, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 200);

                SDL_RenderPresent(Renderer);

                break;
            }
            case State_Game:
            {
                DrawBackgroundAndScores();

                // Paddles & Ball
                SDL_SetRenderDrawColor(Renderer, 40, 140, 40, 255);
                SDL_RenderFillRect(Renderer, &PlayerOne.Rect);
                SDL_RenderFillRect(Renderer, &PlayerTwo.Rect);
                SDL_RenderFillRect(Renderer, &Ball.Rect);

                SDL_RenderPresent(Renderer);

                break;
            }
            case State_Pause:
            {
                DrawBackgroundAndScores();

                SDL_SetRenderDrawColor(Renderer, 10, 100, 10, 255);
                SDL_RenderFillRect(Renderer, &PlayerOne.Rect);
                SDL_RenderFillRect(Renderer, &PlayerTwo.Rect);
                SDL_RenderFillRect(Renderer, &Ball.Rect);

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

#if 0
void EntityUpdate(v2 *Position, v2 *Velocity, v2 *Acceleration, r64 DT)
{
    r32 dt = (r32)DT;
    *Position = 0.5f * (*Acceleration) * (dt * dt) + *Velocity + *Position;
    *Velocity = *Acceleration * dt + *Velocity;
    *Acceleration = {};
}
#endif

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
