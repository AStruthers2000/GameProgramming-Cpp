#include "Game.h"

//Class constructor
Game::Game() {
	mWindow = nullptr;
	mIsRunning = true;
}

bool Game::Initialize() {
	//SDL_Init() takes a single parameter, a bitwise OR of all the SDL subsystems to initialize
	//Some common subsystems are SDL_INIT_AUDIO, SDL_INIT_VIDEO, SDL_INIT_HAPTIC, and SDL_INIT_GAMECONTROLLER
	int sdlResult = SDL_Init(SDL_INIT_VIDEO);
	
	if (sdlResult != 0) {
		//SDL_Log() is a simple way to output messages to the console in SDL. it uses the same syntax as C printf()
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}

	//SDL_CreateWindow() will create a standard window. If the function fails, it returns nullptr.
	//Some possible flags to send for window creation are SDL_WINDOW_FULLSCREEN, SDL_WINDOW_FULLSCREEN_DESKTOP, SDL_WINDOW_OPENGL, and SDL_WINDOW_RESIZABLE
	//Most make sense, but SDL_WINDOW_FULLSCREEN_DESKTOP will use full-screen mode at the current desktop resolution AND ignore width/height parameters in SDL_CreateWindow()
	mWindow = SDL_CreateWindow(
		"Pong - Game Programming in C++",	//Window title
		100,								//Top left x-coordinate of window
		100,								//Top left y-coordinate of window
		1024,								//Width of window
		768,								//Height of window
		0									//Flags (0 for no flags set)
	);

	if (!mWindow) {
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}

	//If we have initialized SDL and created a window, we return true
	return true;
}

void Game::Shutdown() {
	//Destroys the created SDL window
	SDL_DestroyWindow(mWindow);

	//Cleans up all initialized subsystems properly
	SDL_Quit();
}

void Game::ProcessInput() {

}

void Game::UpdateGame() {

}

void Game::GenerateOutput() {
	SDL_Event event;

	//while there are still events in the queue
	while (SDL_PollEvent(&event)) {
		//We will handle different event types here
		switch (event.type) {
		case SDL_QUIT:
			mIsRunning = false;
			break;

		}
	}

	//Gets the current state of the keyboard
	const Uint8* state = SDL_GetKeyboardState(NULL);

	//if escape key is pressed, also quit the game
	if (state[SDL_SCANCODE_ESCAPE]) {
		mIsRunning = false;
	}
}

void Game::RunLoop() {
	while (mIsRunning) {
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

