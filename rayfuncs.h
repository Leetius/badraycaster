#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "define.h"

#ifndef M_PI
#define M_PI (3.14159265358979323846264338327950288)
#endif

typedef struct Projectile
{
    double posX;
    double posY;
    double dirX;
    double dirY;
    double speed;
    bool active;
} Projectile;

void verLine(int x, int y1, int y2, const SDL_Color color, SDL_Renderer *renderer);
void shootProjectile(double cameraPosX, double cameraPosY, double cameraDirX, double cameraDirY, Projectile projectiles[MAX_PROJECTILES]);
void updateProjectiles(Projectile projectiles[MAX_PROJECTILES], int worldMap[MAP_WIDTH][MAP_HEIGHT]);
void renderProjectiles(SDL_Renderer *mainRenderer, Projectile projectiles[MAX_PROJECTILES], int screenWidth, int screenHeight, double cameraPosX, double cameraPosY, double cameraDirX, double cameraDirY, double planeX, double planeY, int worldMap[MAP_WIDTH][MAP_HEIGHT]);
int calculateFps(int *oldTick, double frameTime, int newTick, int oldFps);
int displayFPS(SDL_Renderer *mainRenderer, TTF_Font *font, SDL_Color textColor, int fps);
unsigned int calculateChecksum(const int worldMap[MAP_WIDTH][MAP_HEIGHT]);
void saveMap(int worldMap[MAP_WIDTH][MAP_HEIGHT]);
void loadWorldMapFromFile(int worldMap[MAP_WIDTH][MAP_HEIGHT]);
double calculateFrameTime(unsigned long long *prevCounter, unsigned long long *frequency);
void raycastingAlgorithm(double posX, double posY, double dirX, double dirY, double planeX, double planeY, int worldMap[MAP_WIDTH][MAP_HEIGHT], SDL_Renderer *mainRenderer, SDL_Color *colors);
void handleInput(SDL_Event *event, double *posX, double *posY, double *dirX, double *dirY, double *planeX,
                 double *planeY, int worldMap[MAP_WIDTH][MAP_HEIGHT], double moveSpeed, double mouseSensitivity, bool *isMouseLocked,
                 bool *keys, Projectile *projectiles, bool *run, double frameTime);