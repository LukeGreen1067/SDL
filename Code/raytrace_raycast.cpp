// https://lodev.org/cgtutor/raycasting.html#Untextured_Raycaster_
// Amazing tutorial on Raycasting in general going to use this to get a base for this

// https://fabiensanglard.net/b/gebbdoom.pdf
// Blackbook of how to build Doom can use this for later renderers

// https://github.com/jhhoward/Arduboy3D/releases
// A crazy game built for arduboy that uses 3D raycasting to work

// http://crait.net/download.php?file=CircuitDude.ino
// Cool game that I will be basing code off of to make my engine

//NEED TO REFACTOR CODE TOMMOROW GONNA CRY :(

// Using SDL and standard IO
#include <SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>

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
double posX = 22, posY = 12;
double dirX = -1, dirY = 0;
double planeX = 0, planeY = 0.66;

double time = 0;
double oldTime = 0;

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
	if (keystate[SDL_SCANCODE_W]){}
	if (keystate[SDL_SCANCODE_A]){}
	if (keystate[SDL_SCANCODE_S]){}
	if (keystate[SDL_SCANCODE_D]){}
	if (keystate[SDL_SCANCODE_UP]){gameState = STATE_MENU;}
	if (keystate[SDL_SCANCODE_RIGHT]){gameState = STATE_PLAY;}
	if (keystate[SDL_SCANCODE_LEFT]){gameState = STATE_ABOUT;}
	if (keystate[SDL_SCANCODE_DOWN]){gameState = STATE_QUIT;}
};

void renderScreen()
{
	for (int x = 0; x < SCREEN_WIDTH; x++)
	{
		double cameraX = 2 * x / double(SCREEN_WIDTH) - 1; // x-coordinate in camera space
		double rayDirX = dirX + planeX * cameraX;
		double rayDirY = dirY + planeY * cameraX;
		int mapX = int(posX);
		int mapY = int(posY);

		double sideDistX, sideDistY;

		double deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1 / rayDirX);
		double deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1 / rayDirY);
		double perpWallDist;

		int stepX;
		int stepY;

		int hit = 0; // was there a wall hit?
		int side;	 // was a NS or a EW wall hit?
		if (rayDirX < 0){
			stepX = -1;
			sideDistX = (posX - mapX) * deltaDistX;
		}
		else{
			stepX = 1;
			sideDistX = (mapX + 1.0 - posX) * deltaDistX;
		}
		if (rayDirY < 0){
			stepY = -1;
			sideDistY = (posY - mapY) * deltaDistY;
		}
		else{
			stepY = 1;
			sideDistY = (mapY + 1.0 - posY) * deltaDistY;
		}

		while (hit == 0)
		{
			if (sideDistX < sideDistY){
				sideDistX += deltaDistX;
				mapX += stepX;
				side = 0;
			}
			else{
				sideDistY += deltaDistY;
				mapY += stepY;
				side = 1;
			}
			// Check if ray has hit a wall
			if (map[mapX][mapY] > 0){hit = 1;}

			if (side == 0){perpWallDist = (sideDistX - deltaDistX);}
			else{perpWallDist = (sideDistY - deltaDistY);}

			// Calculate height of line to draw on screen
			int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);

			// calculate lowest and highest pixel to fill in current stripe
			int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
			int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
			if (drawStart < 0){drawStart = 0;}
			if (drawEnd >= SCREEN_HEIGHT){drawEnd = SCREEN_HEIGHT - 1;}


		}
	}
}

void getMenuState(SDL_Surface *screenSurface)
{
	switch (gameState)
	{
	case STATE_MENU:
		SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0x00, 0x00)); // Set colour Red#
		// TODO: Default value should be a load menu screen
		break;
	case STATE_PLAY:
		SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x00, 0xFF, 0x00)); // Set colour Green
		// TODO: Add a renderer here to make raycasting using tutorial
		break;
	case STATE_ABOUT:
		SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0xFF)); // Set colour Blue
		// TODO: Render text to the screen watch this be the worst bit
		break;
	case STATE_QUIT:
		break;
	}
}

int main(int argc, char *args[])
{

	SDL_Window *window = NULL;		   // The window we'll be rendering to
	SDL_Surface *screenSurface = NULL; // The surface contained by the window
	SDL_Renderer *renderer = NULL;
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
