#include "Game.h"
#include <SDL.h>
#include <string>
#include <iostream>

bool Game::Initialize()
{
    int sdlResult = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    if (sdlResult != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    SDL_DisplayMode md;
    SDL_GetDesktopDisplayMode(0, &md);
    native_width = md.w;
    native_height = md.h;

    Uint32 flags =
            SDL_WINDOW_BORDERLESS |
            SDL_WINDOW_INPUT_FOCUS |
            SDL_WINDOW_MOUSE_FOCUS |
            SDL_WINDOW_ALLOW_HIGHDPI |
            SDL_WINDOW_FULLSCREEN_DESKTOP;

    mWindow = SDL_CreateWindow(
            "Multiball!",
            0,
            0,
            native_width/2,
            native_height/2,
            flags
    );

    if(!(flags & SDL_WindowFlags::SDL_WINDOW_FULLSCREEN_DESKTOP))
    {
        native_width = native_width / 2;
        native_height = native_height / 2;
    }


    if(!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mRenderer = SDL_CreateRenderer(
            mWindow,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if(!mRenderer)
    {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return false;
    }

    paddles[0] = {
            {paddle_initial_offset, static_cast<float>(native_height) / 2.f},
            Paddles::Paddle_Speed,
            0,
            Paddles::Paddle_Height,
            Paddles::Paddle_Width,
    };

    paddles[1] = {
            {static_cast<float>(native_width) - paddle_initial_offset, static_cast<float>(native_height) / 2.f},
            Paddles::Paddle_Speed,
            0,
            Paddles::Paddle_Height,
            Paddles::Paddle_Width,
    };

    for(int i = 0; i < INITIAL_BALLS; i++)
        balls.push_back(GenerateNewBall());

    timer_spawnBalls = SDL_AddTimer(10000, SpawnRandomBall_callback, this);

    mIsRunning = true;
    return true;
}

void Game::RunLoop()
{
    while(mIsRunning)
    {
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::Shutdown()
{
    SDL_RemoveTimer(timer_spawnBalls);
    SDL_DestroyWindow(mWindow);
    SDL_DestroyRenderer(mRenderer);
    SDL_Quit();
}

void Game::ProcessInput()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                mIsRunning = false;
                break;
        }
    }

    paddles[0].direction = 0;
    paddles[1].direction = 0;

    const Uint8* state = SDL_GetKeyboardState(nullptr);
    if(state[SDL_SCANCODE_ESCAPE])
    {
        mIsRunning = false;
    }

    //player 1 input
    if(state[SDL_SCANCODE_W])
        paddles[0].direction -= 1;
    if(state[SDL_SCANCODE_S])
        paddles[0].direction += 1;

    //player 2 input
    if(state[SDL_SCANCODE_UP])
        paddles[1].direction -= 1;
    if(state[SDL_SCANCODE_DOWN])
        paddles[1].direction += 1;
}

void Game::UpdateGame()
{
    while(!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 6));

    float deltaTime = static_cast<float>(SDL_GetTicks() - mTicksCount) / 1000.f;
    mTicksCount = SDL_GetTicks();
    if(deltaTime > 0.05f) deltaTime = 0.05f;

    paddles[0].UpdatePaddlePosition(native_height, deltaTime);
    paddles[1].UpdatePaddlePosition(native_height, deltaTime);

    std::vector<int> indices_to_delete;
    for(int i = 0; i < balls.size(); i++)
    {
        int off_screen = balls[i].UpdateBallPosition(paddles, native_width, native_height, deltaTime);
        if(off_screen)
        {
            indices_to_delete.push_back(i);

            if (off_screen == -1)
            {
                score[1]++;
            }
            else
            {
                score[0]++;
            }
        }
    }

    for(const auto& i : indices_to_delete)
    {
        auto it = balls.begin() + i;
        balls.erase(it);
    }
}

void Game::GenerateOutput()
{
    SDL_SetRenderDrawColor(mRenderer,81,65,79,255);

    SDL_RenderClear(mRenderer);

    //draw the entire game

    //render walls
    SDL_SetRenderDrawColor(mRenderer, 235, 186,185, 255);

    SDL_Rect wall_top{0, 0, native_width, Walls::Wall_Thickness};
    SDL_Rect wall_bottom{0, native_height - Walls::Wall_Thickness, native_width, Walls::Wall_Thickness};
    SDL_RenderFillRect(mRenderer, &wall_top);
    SDL_RenderFillRect(mRenderer, &wall_bottom);

    //render balls
    SDL_SetRenderDrawColor(mRenderer, 56, 134, 151, 255);
    for(const auto& ball : balls)
    {
        auto rect = ball.GetBounds();
        SDL_RenderFillRect(mRenderer, &rect);
    }

    //render paddles
    SDL_SetRenderDrawColor(mRenderer, 181, 255, 225, 255);
    auto paddle1 = paddles[0].GetBounds();
    auto paddle2 = paddles[1].GetBounds();
    SDL_RenderFillRect(mRenderer, &paddle1);
    SDL_RenderFillRect(mRenderer, &paddle2);

    SDL_RenderPresent(mRenderer);
}

Uint32 Game::SpawnRandomBall_callback(Uint32 interval, void *param)
{
    Game* g = static_cast<Game*>(param);
    g->SpawnRandomBall_handle();
    return interval;
}

void Game::SpawnRandomBall_handle()
{
    balls.push_back(GenerateNewBall());
}




