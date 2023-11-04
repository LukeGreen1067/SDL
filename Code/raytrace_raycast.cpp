// https://lodev.org/cgtutor/raycasting.html#Untextured_Raycaster_
// Amazing tutorial on Raycasting in general going to use this to get a base for this

// https://fabiensanglard.net/b/gebbdoom.pdf
// Blackbook of how to build Doom can use this for later renderers

// https://github.com/jhhoward/Arduboy3D/releases
// A crazy game built for arduboy that uses 3D raycasting to work

// http://crait.net/download.php?file=CircuitDude.ino
// Cool game that I will be basing code off of to make my engine

// Using SDL and standard IO
#include <SDL.h>
#include <stdio.h>
#include <stdint.h>

// map constants
#define MAPHEIGHT 10
#define MAPWIDTH 10
// Screen dimension constants
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define STATE_MENU 0
#define STATE_PLAY 1
#define STATE_ABOUT 2
#define STATE_QUIT 3

unsigned char gameState = STATE_MENU;

const uint8_t *keystate = SDL_GetKeyboardState(NULL);

int map[MAPHEIGHT][MAPWIDTH] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 2, 2, 0, 0, 0, 1},
	{1, 0, 0, 0, 2, 2, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

void getInput()
{
	// Work out how to make SDL buttons for the game start even if this is just a menu system
	// Will have to split this code to make the game inputs and menu inputs
	if (keystate[SDL_SCANCODE_A])
	{
		gameState = STATE_MENU;
	}
	if (keystate[SDL_SCANCODE_D])
	{
		gameState = STATE_PLAY;
	}

	if (keystate[SDL_SCANCODE_W])
	{
		gameState = STATE_ABOUT;
	}

	if (keystate[SDL_SCANCODE_S])
	{
		gameState = STATE_QUIT;
	}
};

void getMenuState(SDL_Surface *screenSurface)
{
	switch (gameState)
	{
	case STATE_MENU:
		SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0x00, 0x00)); // Set colour Red
		break;
	case STATE_PLAY:
		SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x00, 0xFF, 0x00)); // Set colour Green
		// TODO: Add a renderer here to make raycasting using tutorial
		break;
	case STATE_ABOUT:
		SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0xFF)); // Set colour Blue
		break;
	case STATE_QUIT:
		break;
	}
}

int main(int argc, char *args[])
{

	SDL_Window *window = NULL;		   // The window we'll be rendering to
	SDL_Surface *screenSurface = NULL; // The surface contained by the window
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	// Create window and check for errors tutorial had a pointless else statement which made it look ugly
	window = SDL_CreateWindow("Mining Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
	}
	screenSurface = SDL_GetWindowSurface(window);											// Get window surface
	SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00)); // Fill the surface Black
	SDL_Event e;
	bool quit = false;

	// gameLoop
	while (quit == false)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				quit = true;
		}

		getInput();
		getMenuState(screenSurface);
		SDL_UpdateWindowSurface(window);
	}

	SDL_DestroyWindow(window); // Destroy window
	SDL_Quit();				   // Quit SDL subsystems

	return 0;
}
