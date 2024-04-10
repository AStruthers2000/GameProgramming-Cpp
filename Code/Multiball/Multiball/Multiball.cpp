// Multiball.cpp : Defines the entry point for the application.
//

#include "Game.h"

int main(int argc, char* argv[])
{
	Game game;
    bool success = game.Initialize();
    if(success)
    {
        game.RunLoop();
    }
    game.Shutdown();
	return 0;
}
