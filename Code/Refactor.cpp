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
#define UNITSIZE 64
#define PI 3.14159265359
#define PI2 PI/2
#define PI3 3*PI2
#define DR 0.0174533 

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
    float angle;
} state;

struct {
    int rayAngle, depthOfFeild, mapX, mapY, mapPos;
    float rayX, rayY, xOffset, yOffset, horX, horY, verX, verY, disT;
} ray;

#define STATE_MENU 0
#define STATE_PLAY 1
#define STATE_ABOUT 2
#define STATE_QUIT 3

unsigned char gameState = STATE_PLAY;

const uint8_t *keystate = SDL_GetKeyboardState(NULL);

//------------------------------------------------Main Functions--------------------------------------------------------------------------

int map[MAPHEIGHT * MAPWIDTH]=
{
    1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,2,2,0,0,0,1,
    1,0,0,0,2,2,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1
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

void getInput()
{
	// Work out how to make SDL buttons for the game start even if this is just a menu system
	// Will have to split this code to make the game inputs and menu inputs

    // Move the player forwards and backwards
	if (keystate[SDL_SCANCODE_W]){state.pos.x += state.dir.x; state.pos.y += state.dir.y;}
	if (keystate[SDL_SCANCODE_S]){state.pos.x -= state.dir.x; state.pos.y -= state.dir.y;}

    // Rotate the player direction in radians
	if (keystate[SDL_SCANCODE_D]){        
        state.angle -= 0.1; 
        if(state.angle > 2*PI){ 
            state.angle -= 2*PI;
        } 
        state.dir.x = cos(state.angle) *5; 
        state.dir.y = sin(state.angle) *5; 
    }
	if (keystate[SDL_SCANCODE_A]){
        state.angle -= 0.1; 
        if(state.angle < 0){ 
            state.angle += 2*PI;
        } 
        state.dir.x = cos(state.angle) *5; 
        state.dir.y = sin(state.angle) *5; 
    }
	// if (keystate[SDL_SCANCODE_UP]){gameState = STATE_MENU;}
	// if (keystate[SDL_SCANCODE_RIGHT]){gameState = STATE_PLAY;}
	// if (keystate[SDL_SCANCODE_LEFT]){gameState = STATE_ABOUT;}
	// if (keystate[SDL_SCANCODE_DOWN]){gameState = STATE_QUIT;}
};

float dist(float ax, float ay, float bx, float by, float ang){
    return ( ( (bx-ax)*(bx - ax) + (by-ay) * (by-ay)) ) ;
}

static void verline(int x, int y0, int y1, uint32_t color) {
    for (int y = y0; y <= y1; y++) {
        state.pixels[(y * SCREEN_WIDTH) + x] = color;
    }
}

void DrawRays(){
    ray.rayAngle = state.angle - DR * 30;
    if(ray.rayAngle<0){ray.rayAngle += 2*PI;}
    if(ray.rayAngle>2*PI) {ray.rayAngle -= 2*PI;}
    for(int i = 0; i < 60; i++){
        ray.depthOfFeild = 0;
        float disH = 10000000;
        ray.horX = state.pos.x;
        ray.horY = state.pos.y;
        float aTan=-1/tan(ray.rayAngle);
        if(ray.rayAngle>PI){
            ray.rayY = (((int)state.pos.y >> 6) << 6) - 0.00001; //Get the nearest 64 rounding magic
            ray.rayX = (state.pos.y - ray.rayY) * aTan + state.pos.x;
            ray.yOffset = -64;
            ray.xOffset =-ray.yOffset*aTan; 
        }
        if(ray.rayAngle<PI){
            ray.rayY = (((int)state.pos.y >> 6) << 6) - 0.00001; //Get the nearest 64 rounding magic
            ray.rayX = (state.pos.y - ray.rayY) * aTan + state.pos.x;
            ray.yOffset = -64;
            ray.xOffset =-ray.yOffset*aTan; 
        }
        if(ray.rayAngle == 0 || ray.rayAngle == PI){
            ray.rayX = state.pos.x;
            ray.rayY = state.pos.y;
            ray.depthOfFeild=8;
        }
        while(ray.depthOfFeild < 8){
            ray.mapX = (int) (ray.rayX) >> 6;
            ray.mapY = (int) (ray.rayY) >> 6;
            ray.mapPos = ray.mapY*MAPHEIGHT+ray.mapX;
            if(ray.mapPos > 0 && ray.mapPos < ray.mapX*ray.mapY && map[ray.mapPos] == 1){
                ray.horX=ray.rayX;
                ray.horY=ray.rayY;
                disH = dist(state.pos.x, state.pos.y, ray.horX, ray.horY, ray.rayAngle);
                ray.depthOfFeild = 8;
            }
            else{
            ray.rayX += ray.xOffset;
            ray.rayY += ray.yOffset;
            ray.depthOfFeild+=1;
            }
        }


        ray.depthOfFeild = 0;
        float disV = 10000000;
        ray.verX = state.pos.x;
        ray.verY = state.pos.y;
        float nTan=-tan(ray.rayAngle);
        if(ray.rayAngle>PI2 && ray.rayAngle < PI3){
            ray.rayX = (((int)state.pos.x >> 6) << 6) - 0.00001; //Get the nearest 64 rounding magic
            ray.rayY = (state.pos.y - ray.rayY) * nTan + state.pos.y;
            ray.xOffset = -64;
            ray.yOffset =-ray.xOffset*nTan; 
        }
        if(ray.rayAngle<PI2 || ray.rayAngle>PI3){
            ray.rayX = (((int)state.pos.x >> 6) << 6) + 64; //Get the nearest 64 rounding magic
            ray.rayY = (state.pos.x - ray.rayX) * nTan + state.pos.y;
            ray.xOffset = -64;
            ray.yOffset =-ray.xOffset*nTan; 
        }
        if(ray.rayAngle == 0 || ray.rayAngle == PI){
            ray.rayX = state.pos.x;
            ray.rayY = state.pos.y;
            ray.depthOfFeild=8;
        }
        while(ray.depthOfFeild < 8){
            ray.mapX = (int) (ray.rayX) >> 6;
            ray.mapY = (int) (ray.rayY) >> 6;
            ray.mapPos = ray.mapY*MAPHEIGHT+ray.mapX;
            if(ray.mapPos > 0 && ray.mapPos < ray.mapX*ray.mapY && map[ray.mapPos] == 1){
                ray.verX=ray.rayX;
                ray.verY=ray.rayY;
                disV = dist(state.pos.x, state.pos.y, ray.verX, ray.verY, ray.rayAngle);
                ray.depthOfFeild = 8;
            }
            else{
            ray.rayX += ray.xOffset;
            ray.rayY += ray.yOffset;
            ray.depthOfFeild+=1;
            }
        }
        if(disV<disH) {ray.rayX = ray.verX; ray.rayY= ray.verY; ray.disT = disV;}
        if(disV>disH) {ray.rayX = ray.horX; ray.rayY= ray.horY; ray.disT = disH;}
        ray.rayAngle += DR;
        if(ray.rayAngle<0){ray.rayAngle += 2*PI;}
        if(ray.rayAngle>2*PI) {ray.rayAngle -= 2*PI;}

        float lineH = (UNITSIZE*SCREEN_HEIGHT)/ray.disT; 
        if(lineH>SCREEN_HEIGHT){
            lineH=SCREEN_HEIGHT;
        }
    }
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
        // renderVideo();         
        renderLoop();  
	}

	SDL_DestroyWindow(state.window); // Destroy window
	SDL_DestroyRenderer(state.renderer); // Destroy window
    SDL_Quit();				   // Quit SDL subsystems

	return 0;
}