
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "define.h"
#include "rayfuncs.h"

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		printf("FAIL\n");
	if (TTF_Init() == -1)
	{
		printf("INIT FAILED");
		TTF_GetError();
	}

	int worldMap[MAP_WIDTH][MAP_HEIGHT];
	Projectile projectiles[MAX_PROJECTILES];

	SDL_Event key;
	SDL_Window *mainWindow = SDL_CreateWindow("RAYCASTER", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	SDL_Surface *mainSurface = SDL_GetWindowSurface(mainWindow);
	SDL_Renderer *mainRenderer = SDL_CreateSoftwareRenderer(mainSurface);
	SDL_Color colors[] = {
		{230, 66, 64, 255},	  // Red
		{62, 246, 44, 255},	  // Green
		{45, 150, 255, 255},  // Blue
		{255, 255, 255, 255}, // White
		{255, 252, 33, 255}	  // Yellow
	};

	SDL_Surface *minimapSurface = SDL_CreateRGBSurface(0, MINIMAP_WIDTH - 8, MINIMAP_HEIGHT - 8, 32, 0, 0, 0, 0);
	int cellWidth = MINIMAP_WIDTH / MAP_WIDTH;
	int cellHeight = MINIMAP_HEIGHT / MAP_HEIGHT;
	SDL_Rect minimapRect;
	minimapRect.x = 10;				// X-coordinate of the minimap's top-left corner
	minimapRect.y = 10;				// Y-coordinate of the minimap's top-left corner
	minimapRect.w = MINIMAP_WIDTH;	// Width of the minimap
	minimapRect.h = MINIMAP_HEIGHT; // Height of the minimap

	TTF_Font *font = TTF_OpenFont("vera.ttf", 24);
	if (!font)
	{
		printf("Error loading font: %s\n", TTF_GetError());
	}
	SDL_Color textColor = {255, 255, 255, 255};

	SDL_SetWindowPosition(mainWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	unsigned long long prevCounter = 0;
	unsigned long long frequency = 0;
	int oldTick = 0;

	const double mouseSensitivity = 0.2 / 100;
	bool isMouseLocked = false;

	bool keys[SDL_NUM_SCANCODES] = {false};

	double posX = 22, posY = 12;
	double dirX = -1, dirY = 0;
	double planeX = 0, planeY = 0.66;

	loadWorldMapFromFile(worldMap);

	SDL_SetRenderDrawColor(mainRenderer, 0, 0, 0, 255);
	SDL_RenderClear(mainRenderer);

	for (int i = 0; i < MAX_PROJECTILES; i++)
	{
		if (projectiles[i].active)
		{
			projectiles[i].active = false;
		}
	}

	bool run = true;
	while (run)
	{
		double frameTime = calculateFrameTime(&prevCounter, &frequency);



		raycastingAlgorithm(posX, posY, dirX, dirY, planeX, planeY, worldMap, mainRenderer, colors);
		updateProjectiles(projectiles, worldMap);
		renderProjectiles(mainRenderer, projectiles, SCREEN_WIDTH, SCREEN_HEIGHT, posX, posY, dirX, dirY, planeX, planeY, worldMap);

		
		SDL_FillRect(minimapSurface, NULL, SDL_MapRGB(minimapSurface->format, 255, 255, 255));

		Uint32 wallColors[5]; // Assuming you have 5 different wall colors
		for (int i = 0; i < 5; i++)
		{
			wallColors[i] = SDL_MapRGB(minimapSurface->format, colors[i].r, colors[i].g, colors[i].b);
		}

		for (int y = 0; y < MAP_HEIGHT; y++)
		{
			for (int x = 0; x < MAP_WIDTH; x++)
			{
				int cellValue = worldMap[x][y];
				SDL_Rect cellRect = {(MAP_WIDTH - x - 1) * cellWidth, y * cellHeight, cellWidth, cellHeight};
				SDL_FillRect(minimapSurface, &cellRect, wallColors[cellValue - 1]);
			}
		}

		int playerSize= 20;
		int randColor = rand() % 255;
		int randColor2 = rand() % 255;
		int randColor3 = rand() % 255;
		double minimapPlayerX = ((MAP_WIDTH - posX) / MAP_WIDTH * MINIMAP_WIDTH - (playerSize / 2));
		double minimapPlayerY = (posY / MAP_HEIGHT * MINIMAP_HEIGHT) - playerSize;
		SDL_Rect playerRect = {minimapPlayerX, minimapPlayerY, playerSize, playerSize};							  // Adjust the size of the player marker as needed
		SDL_FillRect(minimapSurface, &playerRect, SDL_MapRGB(minimapSurface->format, 255, 255, 100)); // Use red color for the player marker

		SDL_Texture *minimapTexture = SDL_CreateTextureFromSurface(mainRenderer, minimapSurface);
		SDL_RenderCopy(mainRenderer, minimapTexture, NULL, &minimapRect); // Adjust the position and size of the minimap on the screen
		SDL_DestroyTexture(minimapTexture);

		//int oldFps = displayFPS(mainRenderer, font, textColor, calculateFps(&oldTick, frameTime, SDL_GetTicks(), oldFps));

		double moveSpeed = frameTime * 2.0;
		handleInput(&key, &posX, &posY, &dirX, &dirY, &planeX, &planeY, worldMap, moveSpeed, mouseSensitivity, &isMouseLocked, keys, projectiles, &run, frameTime);

		// Update the window and clear the screen
		SDL_UpdateWindowSurface(mainWindow);
		SDL_SetRenderDrawColor(mainRenderer, 0, 0, 0, 255);
		SDL_RenderClear(mainRenderer);
	}

	// Clean up
	SDL_DestroyRenderer(mainRenderer);
	SDL_DestroyWindow(mainWindow);
	SDL_Quit();

	printf("END\n");
	return 0;
}