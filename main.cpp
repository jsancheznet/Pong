#pragma warning(disable: 4189)
#pragma warning(disable: 4201)
#pragma warning(disable: 4100)

#pragma comment(lib, "Shcore.lib")
#include <windows.h>
#include <ShellScalingAPI.h>
#include <comdef.h>
#include <SDL.h>
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

// TODO: Statically Link Executable

static i32 IsRunning = true;

union color
{
    struct
    {
        r32 r, g, b, a;
    };
};

struct paddle
{
    i32 Side; // 0 == Left, 1 == Right
    color Color;
    SDL_Rect Rect;
    const int Width = 100;
    const int Height = 100;
    const int HalfWidth = Width / 2;
    const int HalfHeight = Height / 2;
};

void MySDL_GetDisplayDPI(i32 DisplayIndex, r32 *DPI, r32 *DefaultDPI)
{
    // High DPI Resources
    // https://seabird.handmade.network/blogs/p/2460-be_aware_of_high_dpi
    // https://nlguillemot.wordpress.com/2016/12/11/high-dpi-rendering/

#ifdef __APPLE__
    const float SystemDefaultDPI = 72.0f;
#elif defined(_WIN32)
    const float SystemDefaultDPI = 96.0f;
#else
    static_assert(false, "No system default DPI set for this platform.");
#endif

    if(SDL_GetDisplayDPI(DisplayIndex, NULL, DPI, NULL) != 0)
    {
        // Failed to get DPI, just return the deault value
        printf("SDL_GetDisplayDPI Failed: %s\n", SDL_GetError());

        if(DPI)
        {
            *DPI = SystemDefaultDPI;
        }
    }

    if(DefaultDPI)
    {
        *DefaultDPI = SystemDefaultDPI;
    }
}

SDL_Window *MySDL_CreateDpiAwareWindow(char *Title, i32 x, i32 y, i32 w, i32 h, u32 Flags)
{
    r32 DPI, DefaultDpi;
    MySDL_GetDisplayDPI(0, &DPI, &DefaultDpi);

    int WindowScaledWidth = (int)(w * DPI / DefaultDpi);
    int WindowScaledHeight = (int)(h * DPI / DefaultDpi);

    SDL_Window *Window = SDL_CreateWindow("Pong",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WindowScaledWidth, WindowScaledHeight,
                                          SDL_WINDOW_ALLOW_HIGHDPI);
    return Window;
}

i32 main(i32 argc, char **argv)
{

    // Enable DPI Awareness
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *Window = MySDL_CreateDpiAwareWindow("Pong",
                                                    SDL_WINDOWPOS_CENTERED,
                                                    SDL_WINDOWPOS_CENTERED,
                                                    640, 480,
                                                    SDL_WINDOW_ALLOW_HIGHDPI);

    SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1,
                                                SDL_RENDERER_ACCELERATED |
                                                SDL_RENDERER_PRESENTVSYNC);

    if(SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND))
    {
        // Error!
        return 0;
    }

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

        // Render
        SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
        SDL_RenderClear(Renderer);

        SDL_SetRenderDrawColor(Renderer, 255, 0, 255, 255);

        SDL_RenderPresent(Renderer);
    }

    return 0;
}
