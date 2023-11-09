// https://www.youtube.com/watch?v=TBGu3NNpF1Q good video on a compiler optimisation might be useful later

#include <SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <cmath>
#include <vector>

//------------------------------------------------Definitions----------------------------------------------------------------------------

#define ASSERT(_e, ...) if (!(_e)) { fprintf(stderr, __VA_ARGS__); exit(1); } 
//found here https://computergraphics.stackexchange.com/questions/13355/undefined-reference-to-sdl-setrenderdrawcolor
//I think the original was https://github.com/jdah/doomenstein-3d/blob/main/src/main_wolf.c#L6

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define MAPHEIGHT 10
#define MAPWIDTH 10

//------------------------------------------------Type/ Struct Definitions----------------------------------------------------------------

typedef struct v2_s { float x, y; } v2;
typedef struct v2i_s { int32_t x, y; } v2i;

struct {
    SDL_Window *window;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
    uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    bool quit;

    v2 pos, dir, plane;
} state;

struct {
    double posX, posY, dirX, dirY, planeX, planeY, time, oldTime, moveSpeed, rotSpeed;
} player;

#define STATE_MENU 0
#define STATE_PLAY 1
#define STATE_ABOUT 2
#define STATE_QUIT 3

unsigned char gameState = STATE_PLAY;

const uint8_t *keystate = SDL_GetKeyboardState(NULL);

//------------------------------------------------Main Functions--------------------------------------------------------------------------

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

//This sets up the code like the reference on https://github.com/jdah/doomenstein-3d/blob/main/src/main_wolf.c
void SDLInit(){
    ASSERT(
        !SDL_Init(SDL_INIT_VIDEO),
        "SDL failed to initialize: %s\n",
        SDL_GetError());
    state.window =
        SDL_CreateWindow(
            "DEMO",
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            SDL_WINDOWPOS_CENTERED_DISPLAY(0),
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_ALLOW_HIGHDPI);
    ASSERT(
        state.window,
        "failed to create SDL window: %s\n", SDL_GetError());

    state.renderer = 
        SDL_CreateRenderer(
            state.window,
            -1,
            SDL_RENDERER_PRESENTVSYNC);
    ASSERT(
        state.renderer,
        "failed to create SDL renderer: %s\n", SDL_GetError());
    state.texture =
        SDL_CreateTexture(
            state.renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH,
            SCREEN_HEIGHT);
    ASSERT(
        state.texture,
        "failed to create SDL texture: %s\n", SDL_GetError());
}

void playerInit(){
    player.posX = 22, player.posY = 12;
    player.dirX = -1, player.dirY = 0;
    player.planeX = 0, player.planeY = 0.66;

    player.time = 0, player.oldTime = 0;
}

void verLine(int x, int y0, int y1, uint32_t color) {// I have no idea how this works 
    for (int y = y0; y <= y1; y++) {
        state.pixels[(y * SCREEN_WIDTH) + x] = color;
    }
}
/*
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
*/

void rotate(double rotSpeed){
        double oldDirX = player.dirX;
        player.dirX = player.dirX * cos(rotSpeed) - player.dirY * sin(rotSpeed);
        player.dirY = oldDirX * sin(rotSpeed) + player.dirY * cos(rotSpeed);
        double oldPlaneX = player.planeX;
        player.planeX = player.planeX * cos(rotSpeed) - player.planeY * sin(rotSpeed);
        player.planeY = oldPlaneX * sin(rotSpeed) + player.planeY * cos(rotSpeed);
}

void move(double dirX, double dirY){
        if(map[int(player.posX + dirX * player.moveSpeed)][int(player.posY)] == false) player.posX += dirX * player.moveSpeed;
        if(map[int(player.posX)][int(player.posY + dirY * player.moveSpeed)] == false) player.posY += dirY * player.moveSpeed;
}

void getInput()
{
	// Work out how to make SDL buttons for the game start even if this is just a menu system
	// Will have to split this code to make the game inputs and menu inputs
	if (keystate[SDL_SCANCODE_W]){move(player.dirX, player.dirY);}
	if (keystate[SDL_SCANCODE_S]){move(-player.dirX, -player.dirY);}
	if (keystate[SDL_SCANCODE_D]){rotate(-player.rotSpeed);}
	if (keystate[SDL_SCANCODE_A]){rotate(player.rotSpeed);}
	if (keystate[SDL_SCANCODE_UP]){gameState = STATE_MENU;}
	if (keystate[SDL_SCANCODE_RIGHT]){gameState = STATE_PLAY;}
	if (keystate[SDL_SCANCODE_LEFT]){gameState = STATE_ABOUT;}
	if (keystate[SDL_SCANCODE_DOWN]){gameState = STATE_QUIT;}
};

void renderVideo(){  
    for(int x = 0; x < SCREEN_WIDTH; x++)
    {
        double cameraX = 2 * x / double(SCREEN_WIDTH) - 1;
        double rayDirX = player.dirX + player.planeX * cameraX;
        double rayDirY = player.dirY + player.planeY * cameraX;

        int mapX = int(player.posX);
        int mapY = int(player.posY);

        double sideDistX;
        double sideDistY;

        double deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1 / rayDirX);
        double deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1 / rayDirY);
        double perpWallDist;

        int stepX;
        int stepY;

        int hit = 0;
        int side;

        if (rayDirX < 0){
            stepX = -1;
            sideDistX = (player.posX - mapX) * deltaDistX;
        }
        else{
            stepX = 1;
            sideDistX = (mapX + 1.0 - player.posX) * deltaDistX;
        }
        if (rayDirY < 0){
            stepY = -1;
            sideDistY = (player.posY - mapY) * deltaDistY;
        }
        else{
            stepY = 1;
            sideDistY = (mapY + 1.0 - player.posY) * deltaDistY;
        }

        if(side == 0) perpWallDist = (sideDistX - deltaDistX);
        else          perpWallDist = (sideDistY - deltaDistY);

        int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);

        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if(drawStart < 0)drawStart = 0;
        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
        if(drawEnd >= SCREEN_HEIGHT)drawEnd = SCREEN_HEIGHT - 1;

        uint32_t colour;
        switch (map[mapX][mapY]) {
        case 1: colour = 0xFF0000FF; break;
        case 2: colour = 0xFF00FF00; break;
        case 3: colour = 0xFFFF0000; break;
        case 4: colour = 0xFFFF00FF; break;
        }
        if (side == 1) {colour = colour / 2;}

        verLine(x, 0, drawStart, 0xFF202020);
        verLine(x, drawStart, drawEnd, colour);
        verLine(x, drawEnd, SCREEN_HEIGHT - 1, 0xFF505050);
    }
    player.oldTime = player.time;
    player.time = SDL_GetTicks();
    double frameTime = (player.time - player.oldTime) / 1000.0; //frameTime is the time this frame has taken, in seconds

    player.moveSpeed = frameTime * 5.0;
    player.rotSpeed = frameTime * 3.0;
}

void renderLoop(){
    SDL_UpdateTexture(state.texture, NULL, state.pixels, SCREEN_WIDTH * 4);
    SDL_RenderCopyEx(
        state.renderer,
        state.texture,
        NULL,
        NULL,
        0.0,
        NULL,
        SDL_FLIP_VERTICAL);
    SDL_RenderPresent(state.renderer);
}

int main(int argc, char *args[])
{
    SDLInit();
    playerInit();

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

        for(int x = 0; x <= SCREEN_HEIGHT; x++){
            verLine(x, 100, 100, 0xFFFF00FF);
        }
        // renderVideo();         
        renderLoop();  
	}

	SDL_DestroyWindow(state.window); // Destroy window
	SDL_DestroyRenderer(state.renderer); // Destroy window
    SDL_Quit();				   // Quit SDL subsystems

	return 0;
}