#include "Game.h"

//the entry point of all c++ programs is the main function. This sets up and starts running the game 
int main(int argc, char** argv) {
	Game game;
	bool success = game.Initialize();

	if (success) {
		//infinite loop until game.mIsRunning == false
		game.RunLoop();
	}

	game.Shutdown();
	return 0;
}