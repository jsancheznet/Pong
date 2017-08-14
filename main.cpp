/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <jsanchez@monoinfinito.net> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 */

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <assert.h>

#include "pong_math.h"

//
// Assets
//

#include "assets/8bit_font.h"
#include "assets/sound_beep_wav.h"
#include "assets/sound_peep_wav.h"
#include "assets/sound_plop_wav.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_CENTER_X (WINDOW_WIDTH / 2)
#define WINDOW_CENTER_Y (WINDOW_HEIGHT / 2)
#define PADDLE_WIDTH 15
#define PADDLE_HEIGHT 99
#define PADDLE_SPEED 50.0f
#define BALL_RADIUS 10
#define TEXT_COLOR {120, 160, 100}
#define SCORE_TEXT_COLOR {10, 70, 10}
#define BALL_INITIAL_SPEED 5.0f
#define BALL_MAX_SPEED 7.0f

enum state
{
    State_Initial,
    State_Game,
    State_Pause,
    State_GameOver
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
    v2 Position;
    v2 Velocity;
    v2 Acceleration;
    v2 Size;
    r32 Speed;
};

struct paddle
{
    r32 Speed;
    i32 Score;
    v2 Position;
    v2 Velocity;
    v2 Acceleration;
    v2 Size;
};

struct gamestate
{
    state CurrentState;

    // Background made of SDL_Rects
    SDL_Rect Background[3] = {};

    // Font
    TTF_Font *FontBig = NULL;
    TTF_Font *FontSmall = NULL;
    SDL_Texture *GameOver = NULL;
    SDL_Texture *Title = NULL;
    SDL_Texture *SpaceToBegin = NULL;
    SDL_Texture *MovementExplanation = NULL;
    SDL_Texture *HumanWon = NULL;
    SDL_Texture *HumanLost = NULL;
    SDL_Texture *PauseTitle = NULL;
    SDL_Texture *PauseContinue = NULL;
    SDL_Texture *EscapeToExit = NULL;

    // Sounds
    Mix_Chunk *Beep = NULL;
    Mix_Chunk *Peep = NULL;
    Mix_Chunk *Plop = NULL;

    // Time Related
    r64 DeltaTime;
    u64 FrameStart;
    u64 FrameEnd;
    u64 CounterFreq;
    r64 SecondsPassed = NULL;
    u64 FrameCounter = NULL;
    u64 FPS = NULL;
};

//
// Function List
//

i32 PlaySound(Mix_Chunk *Chunk);
SDL_Texture *CreateTextureFromText(const char *String, TTF_Font *Font, SDL_Color Color);
int AssetsLoad(void);
void DrawBackground();
void DrawTextureCentered(SDL_Texture *Texture, i32 x, i32 y);
void DrawBall(void);
void DrawScores();
void DrawPaddle(paddle *Paddle);
void DoNewtonMotion(v2 *Position, v2 *Velocity, v2 *Acceleration, r64 DT);
void BallReset();
void UpdateBall(r64 dt);
void UpdatePlayer(r64 dt);
b32 Overlapping(r32 MinA, r32 MaxA, r32 MinB, r32 MaxB);
r32 Penetration(r32 CenterA, r32 SizeA, r32 CenterB, r32 SizeB);
void ResolveCollisions();
void Init();

//
// Globals
//

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

i32 PlaySound(Mix_Chunk *Chunk)
{
    if( Mix_PlayChannel( -1, Chunk, 0 ) == -1 )
    {
        return 1;
    }

    return 0;
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

int AssetsLoad(void)
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
    Gamestate.SpaceToBegin = CreateTextureFromText("Press space to begin", Gamestate.FontSmall, TEXT_COLOR);
    Gamestate.MovementExplanation = CreateTextureFromText("W  S  to control paddle", Gamestate.FontSmall, TEXT_COLOR);
    Gamestate.GameOver = CreateTextureFromText("Game Over", Gamestate.FontBig, TEXT_COLOR);
    Gamestate.HumanWon = CreateTextureFromText("Human won", Gamestate.FontBig, TEXT_COLOR);
    Gamestate.HumanLost = CreateTextureFromText("AI won", Gamestate.FontBig, TEXT_COLOR);
    Gamestate.PauseTitle = CreateTextureFromText("Game Paused", Gamestate.FontBig, TEXT_COLOR);
    Gamestate.PauseContinue = CreateTextureFromText("Press space to continue", Gamestate.FontSmall, TEXT_COLOR);
    Gamestate.EscapeToExit = CreateTextureFromText("Press escape to exit", Gamestate.FontSmall, TEXT_COLOR);

    // Setup Background comprised of SDL_Rects
    const i32 BgLineSpan = 10;
    Gamestate.Background[0] = {0, 0, WINDOW_WIDTH, BgLineSpan};                                    // Background Top Line
    Gamestate.Background[1] = {0, WINDOW_HEIGHT - BgLineSpan, WINDOW_WIDTH, BgLineSpan};           // Background Bottom Line
    Gamestate.Background[2] = {WINDOW_WIDTH / 2 - (BgLineSpan / 2), 0, BgLineSpan, WINDOW_HEIGHT}; // Background Mid Line

    return true;
}

void DrawBackground()
{
    SDL_SetRenderDrawColor(Renderer, 10, 100, 30, 255);
    SDL_RenderFillRects(Renderer, Gamestate.Background, 3);
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

void DrawBall(void)
{
    // Draws a Rectangle from the center
    SDL_Rect Rect;
    Rect.x = (i32)(Ball.Position.x - Ball.Size.x / 2);
    Rect.y = (i32)(Ball.Position.y - Ball.Size.y / 2);
    Rect.w = (i32)Ball.Size.x;
    Rect.h = (i32)Ball.Size.y;
    SDL_SetRenderDrawColor(Renderer, 10, 100, 30, 255);
    SDL_RenderFillRect(Renderer, &Rect);
}

void DrawScores()
{
    // TODO: We can draw one texture with both scores and draw it centered
    char ScoreP1[3] = {};
    char ScoreP2[3] = {};

    _itoa_s(PlayerOne.Score, ScoreP1, 3, 10);
    _itoa_s(PlayerTwo.Score, ScoreP2, 3, 10);

    SDL_Texture *TextureP1 =  CreateTextureFromText(ScoreP1, Gamestate.FontBig, SCORE_TEXT_COLOR);
    SDL_Texture *TextureP2 =  CreateTextureFromText(ScoreP2, Gamestate.FontBig, SCORE_TEXT_COLOR);

    if(PlayerOne.Position.x <= WINDOW_CENTER_X)
    {
        // Its on the left side
        // Draw P1 on left, p2 on right
        DrawTextureCentered(TextureP1, WINDOW_CENTER_X - 100, WINDOW_CENTER_Y);
        DrawTextureCentered(TextureP2, WINDOW_CENTER_X + 100, WINDOW_CENTER_Y);
    }
    else
    {
        // Its on the right side
        // Draw p2 on left, p1 on right
        DrawTextureCentered(TextureP1, WINDOW_CENTER_X + 100, WINDOW_CENTER_Y);
        DrawTextureCentered(TextureP2, WINDOW_CENTER_X - 100, WINDOW_CENTER_Y);
    }

    SDL_DestroyTexture(TextureP1);
    SDL_DestroyTexture(TextureP2);
}

void DrawPaddle(paddle *Paddle)
{
    SDL_Rect Rect;
    Rect.x = (i32)(Paddle->Position.x - Paddle->Size.x / 2);
    Rect.y = (i32)(Paddle->Position.y - Paddle->Size.y / 2);
    Rect.w = (i32)Paddle->Size.x;
    Rect.h = (i32)Paddle->Size.y;
    SDL_SetRenderDrawColor(Renderer, 10, 100, 30, 255);
    SDL_RenderFillRect(Renderer, &Rect);
}

void DoNewtonMotion(v2 *Position, v2 *Velocity, v2 *Acceleration, r64 DT)
{
    r32 dt = (r32)DT;
    *Position = 0.5f * (*Acceleration) * (dt * dt) + *Velocity + *Position;
    *Velocity = *Acceleration * dt + *Velocity;
    *Acceleration = {};
}

void BallReset()
{
    Ball.Position = V2(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    Ball.Velocity = {};
}

void UpdateBall(r64 dt)
{
    // Ball Update
    DoNewtonMotion(&Ball.Position, &Ball.Velocity, &Ball.Acceleration, dt);

    Clamp(&Ball.Velocity.x, -BALL_MAX_SPEED, BALL_MAX_SPEED);
    Clamp(&Ball.Velocity.y, -BALL_MAX_SPEED, BALL_MAX_SPEED);

    //
    // Ball Walls
    //

    // Y positive downwards
    // X Positive right
    r32 MaxX = Ball.Position.x + Ball.Size.x / 2.0f;
    r32 MinX = Ball.Position.x - Ball.Size.x / 2.0f;
    r32 MaxY = Ball.Position.y + Ball.Size.y / 2.0f;
    r32 MinY = Ball.Position.y - Ball.Size.y / 2.0f;

    // Resolve Wall on X
    if(MinX <= 0)
    {
        // Player Two Scored, Update Scores, Reset Ball, Ball goes to Player Two
        PlayerTwo.Score++;
        BallReset();
        PlaySound(Gamestate.Peep);
        Ball.Acceleration = V2(Ball.Speed, 0.0f);
    }
    else if(MaxX >= WINDOW_WIDTH)
    {
        // Player One Scored, Update Scores, Reset Ball, Ball goes to Player One
        PlayerOne.Score++;
        BallReset();
        PlaySound(Gamestate.Peep);
        Ball.Acceleration = V2(-Ball.Speed, 0.0f);
    }
    // Resolve Wall on Y
    if(MaxY >= WINDOW_HEIGHT)
    {
        Ball.Velocity.y *= -1.0f;
        PlaySound(Gamestate.Beep);
    }
    else if(MinY <= 0)
    {
        Ball.Velocity.y *= -1.0f;
        PlaySound(Gamestate.Beep);
    }

}

void UpdatePlayer(r64 dt)
{
    if(Keyboard.State[SDL_SCANCODE_W])
    {
        // Up
        PlayerOne.Acceleration.y -= PADDLE_SPEED;
    }

    if(Keyboard.State[SDL_SCANCODE_S])
    {
        // Down
        PlayerOne.Acceleration.y += PADDLE_SPEED;
    }

    DoNewtonMotion(&PlayerOne.Position, &PlayerOne.Velocity, &PlayerOne.Acceleration, dt);
    PlayerOne.Velocity *= 0.8f; // Calculate Drag

    // Wall Collision on Y
    // MinY Goes up
    // MaxY Goes down
    r32 MinY = PlayerOne.Position.y - PlayerOne.Size.y / 2;
    r32 MaxY = PlayerOne.Position.y + PlayerOne.Size.y / 2;

    if(MinY <= 0)
    {
        PlayerOne.Position.y = PlayerOne.Size.y / 2;
        PlayerOne.Velocity.y = 0.0f;
    }
    else if(MaxY >= WINDOW_HEIGHT)
    {
        PlayerOne.Position.y = WINDOW_HEIGHT - PlayerOne.Size.y / 2;
        PlayerOne.Velocity.y = 0.0f;
    }
}

void UpdateAI(r64 dt)
{
    ball InvisibleBall = Ball;

    if(Ball.Velocity.x > 0)
    {
        while(InvisibleBall.Position.x < WINDOW_WIDTH / 2)
        {
            DoNewtonMotion(&InvisibleBall.Position,
                           &InvisibleBall.Velocity,
                           &InvisibleBall.Acceleration,
                           dt);
        }

        if(PlayerTwo.Position.y > InvisibleBall.Position.y)
        {
            // Move Up
            PlayerTwo.Acceleration.y -= 50.0f;
        }
        else if(PlayerTwo.Position.y < InvisibleBall.Position.y)
        {
            // Move Down
            PlayerTwo.Acceleration.y += 50.0f;
        }
    }

    DoNewtonMotion(&PlayerTwo.Position,
                   &PlayerTwo.Velocity,
                   &PlayerTwo.Acceleration,
                   dt);
    PlayerTwo.Velocity *= 0.8f; // Calculate Drag

    // Wall Collision on Y
    // MinY Goes up
    // MaxY Goes down
    r32 MinY = PlayerTwo.Position.y - PlayerTwo.Size.y / 2;
    r32 MaxY = PlayerTwo.Position.y + PlayerTwo.Size.y / 2;

    if(MinY <= 0)
    {
        PlayerTwo.Position.y = PlayerTwo.Size.y / 2;
        PlayerTwo.Velocity.y = 0.0f;
    }
    else if(MaxY >= WINDOW_HEIGHT)
    {
        PlayerTwo.Position.y = WINDOW_HEIGHT - PlayerTwo.Size.y / 2;
        PlayerTwo.Velocity.y = 0.0f;
    }
}

b32 Overlapping(r32 MinA, r32 MaxA, r32 MinB, r32 MaxB)
{
    return MinB <= MaxA && MinA <= MaxB;
}

r32 Penetration(r32 CenterA, r32 SizeA, r32 CenterB, r32 SizeB)
{
    // Penetration Calculation

    // To Make sense of the variable names check the url
    // AABB Explanation at the following URL:
    // http://www.metanetsoftware.com/technique/tutorialA.html

    r32 Result;
    // Red is A
    // Blue is B
    r32 Green = (r32)fabs(CenterA - CenterB);
    r32 Red = (r32)fabs(CenterA - SizeA);
    r32 Blue = (r32)fabs(CenterB - SizeB);
    Result = (r32)fabs((Red + Blue) - Green);

    return (Result);
}

void ResolveCollisions()
{
    /*
      AABB Collision test with each paddle if they collide calculate
      the penetration of the collision using the guide here:
      http://www.metanetsoftware.com/technique/tutorialA.html .Check
      which penetration is smaller.  If the vertical penetration is
      smaller it means it collided on the top or bottom of the paddle,
      otherwise it collided on the front of the paddle
     */

    //
    // Ball vs Left Paddle
    //

    // MinY is Upwards
    // MaxY is Downwards
    r32 P1MinX = PlayerOne.Position.x - PlayerOne.Size.x / 2;
    r32 P1MaxX = PlayerOne.Position.x + PlayerOne.Size.x / 2;
    r32 P1MinY = PlayerOne.Position.y - PlayerOne.Size.y / 2;
    r32 P1MaxY = PlayerOne.Position.y + PlayerOne.Size.y / 2;

    // MinY is Upwards
    // MaxY is Downwards
    r32 BallMinX = Ball.Position.x - Ball.Size.x / 2;
    r32 BallMaxX = Ball.Position.x + Ball.Size.x / 2;
    r32 BallMinY = Ball.Position.y - Ball.Size.y / 2;
    r32 BallMaxY = Ball.Position.y + Ball.Size.y / 2;

    if(Overlapping(BallMinX, BallMaxX, P1MinX, P1MaxX) &&
       Overlapping(BallMinY, BallMaxY, P1MinY, P1MaxY))
    {
        PlaySound(Gamestate.Plop);

        r32 HorizontalPenetration = Penetration(PlayerOne.Position.x,
                                                PlayerOne.Position.x + PlayerOne.Size.x / 2,
                                                Ball.Position.x,
                                                Ball.Position.x + Ball.Size.x / 2);

        r32 VerticalPenetration = Penetration(PlayerOne.Position.y,
                                              PlayerOne.Position.y + PlayerOne.Size.y / 2,
                                              Ball.Position.y,
                                              Ball.Position.y + Ball.Size.y / 2);

        if(HorizontalPenetration < VerticalPenetration)
        {
            // This is always PlayerOne, so we can just add HorizontalPenetration
            Ball.Position.x += HorizontalPenetration;
            Ball.Velocity.x *= -1.1f;
        }
        else
        {
            // This is always PlayerOne, so we can just add/subtract VerticalPenetration
            if(Ball.Position.y < PlayerOne.Position.y)
            {
                // Ball Collided on the top of the paddle
                Ball.Position.y -= VerticalPenetration;
            }
            else
            {
                // Ball collided on the bottom of the paddle
                Ball.Position.y += VerticalPenetration;
            }

        }

        // Modify the Angle of the ball if it hit the correct places
        if(Ball.Position.y  - PlayerOne.Position.y < 0)
        {
            Ball.Acceleration.y -= 100.0f;
        }
        else if(Ball.Position.y - PlayerOne.Position.y > 0)
        {
            Ball.Acceleration.y += 100.0f;
        }

    }

    //
    // Ball Vs Right Paddle
    //

    // MinY is Upwards
    // MaxY is Downwards
    r32 P2MinX = PlayerTwo.Position.x - PlayerTwo.Size.x / 2;
    r32 P2MaxX = PlayerTwo.Position.x + PlayerTwo.Size.x / 2;
    r32 P2MinY = PlayerTwo.Position.y - PlayerTwo.Size.y / 2;
    r32 P2MaxY = PlayerTwo.Position.y + PlayerTwo.Size.y / 2;

    if(Overlapping(BallMinX, BallMaxX, P2MinX, P2MaxX) &&
       Overlapping(BallMinY, BallMaxY, P2MinY, P2MaxY))
    {
        PlaySound(Gamestate.Plop);
        r32 HorizontalPenetration = Penetration(PlayerTwo.Position.x,
                                                PlayerTwo.Position.x + PlayerTwo.Size.x / 2,
                                                Ball.Position.x,
                                                Ball.Position.x + Ball.Size.x / 2);

        r32 VerticalPenetration = Penetration(PlayerTwo.Position.y,
                                              PlayerTwo.Position.y + PlayerTwo.Size.y / 2,
                                              Ball.Position.y,
                                              Ball.Position.y + Ball.Size.y / 2);

        if(HorizontalPenetration < VerticalPenetration)
        {
            // This is always PlayerTwo, so we can just subtract HorizontalPenetration
            Ball.Position.x -= HorizontalPenetration;
            Ball.Velocity.x *= -1.1f;
        }
        else
        {
            // This is always PlayerOne, so we can just add/subtract VerticalPenetration
            if(Ball.Position.y < PlayerTwo.Position.y)
            {
                // Ball Collided on the top of the paddle
                Ball.Position.y -= VerticalPenetration;
            }
            else
            {
                // Ball collided on the bottom of the paddle
                Ball.Position.y += VerticalPenetration;
            }
            Ball.Velocity.y *= -1.1f;
        }
    }

    Clamp(&Ball.Velocity.y, -12.0f, 12.0f);
    Clamp(&Ball.Velocity.x, -12.0f, 12.0f);
}

void Init()
{
    if(SDL_Init(SDL_INIT_EVERYTHING))
    {
        printf("Error Initializing SDL: %s\n", SDL_GetError());
    }

    if(TTF_Init())
    {
        printf("Error Initializing TTF: %s\n", TTF_GetError());
    }

    if(Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG)
    {
        printf("Error Initializing Mixer: %s\n", Mix_GetError());
    }

    Window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT, NULL);
    if(Window == NULL)
    {
        printf("Error Creating Window: %s\n", SDL_GetError());
    }

    Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(Renderer == NULL)
    {
        printf("Failed creating Renderer, aborting\n");
    }
    SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);

    //Initialize SDL_mixer
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
    }
    // Set volume to 1/8 max volume
    Mix_Volume(-1, MIX_MAX_VOLUME / 10);
}

i32 main(i32 argc, char **argv)
{
    Init();

    AssetsLoad();

    // Ball always begins movement towards player
    Ball.Position = V2(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    Ball.Speed = 300.0f;
    Ball.Size = V2(20.0f, 20.0f);
    Ball.Acceleration = V2(-Ball.Speed, 0.0f);

    // Paddle One
    PlayerOne.Position = V2(20.0f, WINDOW_HEIGHT / 2);
    PlayerOne.Size = V2(20.0f, 100.0f);
    PlayerOne.Speed = PADDLE_SPEED;
    PlayerOne.Score = 0;

    // Paddle Two
    PlayerTwo.Position = V2(WINDOW_WIDTH - 20.0f, WINDOW_HEIGHT / 2);
    PlayerTwo.Size = V2(20.0f, 100.0f);
    PlayerTwo.Speed = PADDLE_SPEED;
    PlayerTwo.Score = 0;

    // Set Currente State
    Gamestate.CurrentState = State_Initial;

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

            if (Event.type == SDL_WINDOWEVENT)
            {
                switch (Event.window.event)
                {
                    case SDL_WINDOWEVENT_HIDDEN:
                    case SDL_WINDOWEVENT_MOVED:
                    case SDL_WINDOWEVENT_MINIMIZED:
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                    {
                        Gamestate.CurrentState = State_Pause;
                        break;
                    }
                }
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
            case State_Initial:
            {
                if(Keyboard.State[SDL_SCANCODE_SPACE])
                {
                    Gamestate.CurrentState = State_Game;
                }

                if(Keyboard.State[SDL_SCANCODE_ESCAPE])
                {
                    IsRunning = false;
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

                UpdateBall(DeltaTime);
                UpdatePlayer(DeltaTime);
                UpdateAI(DeltaTime);

                // Calculate collisions
                ResolveCollisions();

                if(PlayerOne.Score == 10 || PlayerTwo.Score == 10)
                {
                    Gamestate.CurrentState = State_GameOver;
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

                // Space switches back to game
                if(Keyboard.State[SDL_SCANCODE_SPACE])
                {
                    Gamestate.CurrentState = State_Game;
                }

                break;
            }

            case State_GameOver:
            {
                // Escape exits game
                if(Keyboard.State[SDL_SCANCODE_ESCAPE] && !Keyboard.PrevState[SDL_SCANCODE_ESCAPE])
                {
                    IsRunning = false;
                }

                break;
           }
        }

        //
        // Render
        //

        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
        SDL_RenderClear(Renderer);

        switch(Gamestate.CurrentState)
        {
            case State_Initial:
            {
                DrawBackground();
                DrawScores();
                DrawPaddle(&PlayerOne);
                DrawPaddle(&PlayerTwo);
                DrawBall();

                // Draw Text
                DrawTextureCentered(Gamestate.Title, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 200);
                DrawTextureCentered(Gamestate.SpaceToBegin, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
                DrawTextureCentered(Gamestate.MovementExplanation, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 50);
                DrawTextureCentered(Gamestate.EscapeToExit, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 200);

                break;
            }
            case State_Game:
            {
                DrawBackground();
                DrawScores();
                DrawBall();
                DrawPaddle(&PlayerOne);
                DrawPaddle(&PlayerTwo);
                break;
            }
            case State_Pause:
            {
                DrawBackground();
                DrawScores();
                DrawBall();
                DrawPaddle(&PlayerTwo);
                DrawPaddle(&PlayerOne);

                DrawTextureCentered(Gamestate.PauseTitle, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 200);
                DrawTextureCentered(Gamestate.PauseContinue, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 100);
                DrawTextureCentered(Gamestate.EscapeToExit, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 200);

                break;
            }
            case State_GameOver:
            {
                DrawBackground();
                DrawScores();
                DrawBall();
                DrawPaddle(&PlayerTwo);
                DrawPaddle(&PlayerOne);

                if(PlayerOne.Score == 10)
                {
                    // Human Won
                    DrawTextureCentered(Gamestate.HumanWon, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
                }
                else
                {
                    // AI Won
                    DrawTextureCentered(Gamestate.HumanLost, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
                }

                DrawTextureCentered(Gamestate.EscapeToExit, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 200);

                break;
            }
        }
        SDL_RenderPresent(Renderer);

        Gamestate.FrameEnd = SDL_GetPerformanceCounter();
        DeltaTime = (r64)((Gamestate.FrameEnd - Gamestate.FrameStart) * 1000.0f) / Gamestate.CounterFreq;
        DeltaTime /= 1000.0f;
        Gamestate.SecondsPassed += DeltaTime;
        if(Gamestate.SecondsPassed > 1.0f)
        {
            char Title[30] = {};
            sprintf_s(Title, sizeof(Title),"Pong - %lld FPS", Gamestate.FrameCounter);
            SDL_SetWindowTitle(Window, Title);
            Gamestate.SecondsPassed = 0.0f;
            Gamestate.FrameCounter = 0;
        }
        Gamestate.FrameCounter++;
    }

    return 0;
}
