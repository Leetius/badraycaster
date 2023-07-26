#include "rayfuncs.h"

void verLine(int x, int y1, int y2, const SDL_Color color, SDL_Renderer *renderer)
{
    if (y2 < y1)
    {
        y1 += y2;
        y2 = y1 - y2;
        y1 -= y2;
    }

    if (y2 < 0 || y1 >= SCREEN_HEIGHT || x < 0 || x >= SCREEN_WIDTH)
        return;
    if (y1 < 0)
        y1 = 0;
    if (y2 >= SCREEN_HEIGHT)
        y2 = SCREEN_HEIGHT - 1;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    SDL_Point points[] = {{x, y1}, {x, y2}};
    SDL_RenderDrawLines(renderer, points, 2);
}

void shootProjectile(double cameraPosX, double cameraPosY, double cameraDirX, double cameraDirY, Projectile projectiles[MAX_PROJECTILES])
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (!projectiles[i].active)
        {
            projectiles[i].posX = cameraPosX;
            projectiles[i].posY = cameraPosY;
            projectiles[i].dirX = cameraDirX;
            projectiles[i].dirY = cameraDirY;
            projectiles[i].speed = 0.001; // Adjust the speed as needed
            projectiles[i].active = true;
            break;
        }
    }
}

bool isProjectileVisible(double projectileX, double projectileY, double cameraX, double cameraY, int worldMap[MAP_WIDTH][MAP_HEIGHT])
{
    double dx = projectileX - cameraX;
    double dy = projectileY - cameraY;
    double distanceToProjectile = sqrt(dx * dx + dy * dy);

    // Calculate the normalized direction vector from camera to projectile
    double stepX = dx / distanceToProjectile;
    double stepY = dy / distanceToProjectile;

    double x = cameraX;
    double y = cameraY;

    // Set the initial step size based on the distance to the projectile
    double stepSize = 0.1; // Adjust this value as needed

    // Perform raycasting from the camera towards the projectile
    while (distanceToProjectile > 0)
    {
        // Take a step based on the dynamically calculated step size
        x += stepX * stepSize;
        y += stepY * stepSize;
        distanceToProjectile -= stepSize; // Update the remaining distance

        int mapX = (int)(x);
        int mapY = (int)(y);

        // Check if the current position is inside a wall
        if (worldMap[mapX][mapY] > 0)
        {
            return false; // Projectile is behind a wall, not visible
        }

        // Adjust the step size based on the distance to the projectile
        stepSize = 0.1 + 0.09 * (distanceToProjectile / sqrt(dx * dx + dy * dy));
        // Adjust the above coefficient (0.9) as needed for more accurate raycasting
    }

    return true; // Projectile is visible
}

// Function to update projectiles
void updateProjectiles(Projectile projectiles[MAX_PROJECTILES], int worldMap[MAP_WIDTH][MAP_HEIGHT])
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (projectiles[i].active)
        {
            // Move the projectile
            projectiles[i].posX += projectiles[i].dirX * projectiles[i].speed;
            projectiles[i].posY += projectiles[i].dirY * projectiles[i].speed;

            // Check for collisions with walls
            int mapX = (int)(projectiles[i].posX);
            int mapY = (int)(projectiles[i].posY);
            if (worldMap[mapX][mapY] > 0)
            {
                projectiles[i].active = false; // Deactivate the projectile if it hits a wall
            }
            printf("posX: %f, posY: %f\n", projectiles[i].posX, projectiles[i].posY);
        }
    }
}

void renderProjectiles(SDL_Renderer *mainRenderer, Projectile projectiles[MAX_PROJECTILES], int screenWidth, int screenHeight, double cameraPosX, double cameraPosY, double cameraDirX, double cameraDirY, double planeX, double planeY, int worldMap[MAP_WIDTH][MAP_HEIGHT])
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (projectiles[i].active && isProjectileVisible(projectiles[i].posX, projectiles[i].posY, cameraPosX, cameraPosY, worldMap))
        {
            // Calculate the vector from the camera to the projectile
            double dx = projectiles[i].posX - cameraPosX;
            double dy = projectiles[i].posY - cameraPosY;
            double distance = sqrt(dx * dx + dy * dy);

            // Calculate the angle between the camera direction and the projectile vector
            double angle = atan2(cameraDirY, cameraDirX) - atan2(dy, dx);

            // Correct the angle to be between -PI and PI
            while (angle > M_PI)
                angle -= 2 * M_PI;
            while (angle < -M_PI)
                angle += 2 * M_PI;

            // Calculate the perpendicular distance to the projectile (the distance from the camera to the projection plane)
            double perpDistance = distance * cos(angle);

            // Calculate the height of the projectile based on the distance (smaller the farther)
            int projectileHeight = (int)(screenHeight / perpDistance);

            // Calculate the vertical position on the screen
            int screenY = (screenHeight - projectileHeight) / 2;

            // Calculate the horizontal position on the screen (angle-based)
            int screenX = (int)((angle / (33 * M_PI / 180)) * (screenWidth / 2)) + screenWidth / 2;

            // Calculate the percentage of the projectile that is visible
            double visibilityPercentage = 1.0;
            if (perpDistance < distance) // Projectile partially hidden horizontally by a wall
            {
                visibilityPercentage = perpDistance / distance;
            }

            // Calculate the horizontal clipping of the projectile
            int halfProjectileWidth = 4; // Adjust this value to control the width of the projectile
            int clipWidth = (int)(halfProjectileWidth * visibilityPercentage);
            int clipX = halfProjectileWidth - clipWidth;

            // Draw the clipped projectile
            SDL_Rect projectileRect;
            projectileRect.x = screenX + clipX;
            projectileRect.y = screenY;
            projectileRect.w = 2 * clipWidth;
            projectileRect.h = projectileHeight;

            // Draw a filled rectangle for the projectile
            SDL_SetRenderDrawColor(mainRenderer, 255, 0, 255, 255);
            SDL_RenderFillRect(mainRenderer, &projectileRect);
        }
    }
}

int calculateFps(int *oldTick, double frameTime, int newTick, int oldFps)
{
    if ((*oldTick + 100) <= newTick)
    {
        *oldTick = newTick;
        return 1.0 / frameTime;
    }

    return oldFps;
}

int displayFPS(SDL_Renderer *mainRenderer, TTF_Font *font, SDL_Color textColor, int fps)
{
    // Create the FPS text surface
    char fpsText[16];
    snprintf(fpsText, sizeof(fpsText), "FPS: %d", fps);
    SDL_Surface *fpsSurface = TTF_RenderText_Solid(font, fpsText, textColor);

    // Create the FPS texture and free the surface
    SDL_Texture *fpsTexture = SDL_CreateTextureFromSurface(mainRenderer, fpsSurface);
    SDL_FreeSurface(fpsSurface);

    // Get the dimensions of the FPS texture
    int fpsTextWidth, fpsTextHeight;
    SDL_QueryTexture(fpsTexture, NULL, NULL, &fpsTextWidth, &fpsTextHeight);

    // Draw a background rectangle
    SDL_SetRenderDrawBlendMode(mainRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(mainRenderer, 0, 0, 0, 127);
    SDL_Rect backgroundRect = {10, 10, fpsTextWidth + 20, fpsTextHeight + 20};
    SDL_RenderFillRect(mainRenderer, &backgroundRect);
    SDL_SetRenderDrawBlendMode(mainRenderer, SDL_BLENDMODE_NONE);

    // Draw the FPS texture
    SDL_Rect fpsRect = {10, 10, fpsTextWidth, fpsTextHeight};
    SDL_RenderCopy(mainRenderer, fpsTexture, NULL, &fpsRect);

    // Destroy the FPS texture
    SDL_DestroyTexture(fpsTexture);

    return fps;
}

unsigned int calculateChecksum(const int worldMap[MAP_WIDTH][MAP_HEIGHT])
{
    unsigned int checksum = 0;
    for (int i = 0; i < MAP_WIDTH; i++)
    {
        for (int j = 0; j < MAP_HEIGHT; j++)
        {
            checksum ^= worldMap[i][j];
        }
    }
    return checksum;
}

void saveMap(int worldMap[MAP_WIDTH][MAP_HEIGHT])
{
    FILE *file = fopen("worldMap.txt", "w");
    if (file)
    {
        for (int i = 0; i < MAP_WIDTH; i++)
        {
            for (int j = 0; j < MAP_HEIGHT; j++)
            {
                fprintf(file, "%d ", worldMap[i][j]);
            }
            fprintf(file, "\n");
        }
        fclose(file);
    }
    else
    {
        printf("Failed to open the file for writing.\n");
    }
}

void loadWorldMapFromFile(int worldMap[MAP_WIDTH][MAP_HEIGHT])
{
    FILE *file = fopen("worldMap.txt", "r");
    if (!file)
    {
    }

    // Read the content of the file and calculate its checksum
    int fileWorldMap[MAP_WIDTH][MAP_HEIGHT];
    for (int i = 0; i < MAP_WIDTH; i++)
    {
        for (int j = 0; j < MAP_HEIGHT; j++)
        {
            if (fscanf(file, "%d", &fileWorldMap[i][j]) != 1)
            {
                // Error reading the file
                fclose(file);
            }
        }
    }
    fclose(file);

    // Calculate the checksum of the file's content
    unsigned int currentChecksum = calculateChecksum(worldMap);
    unsigned int fileChecksum = calculateChecksum(fileWorldMap);

    // Compare the checksums to see if the file content is different
    if (currentChecksum != fileChecksum)
    {
        for (int i = 0; i < MAP_WIDTH; i++)
        {
            for (int j = 0; j < MAP_HEIGHT; j++)
            {
                worldMap[i][j] = fileWorldMap[i][j];
            }
        }
    }
}

double calculateFrameTime(unsigned long long *prevCounter, unsigned long long *frequency)
{
    unsigned long long currentCounter = SDL_GetPerformanceCounter();

    if (*frequency == 0)
    {
        *frequency = SDL_GetPerformanceFrequency();
        *prevCounter = currentCounter;
    }

    double frameTime = (double)(currentCounter - *prevCounter) / *frequency;
    *prevCounter = currentCounter;

    return frameTime;
}

void raycastingAlgorithm(double posX, double posY, double dirX, double dirY, double planeX, double planeY, int worldMap[MAP_WIDTH][MAP_HEIGHT], SDL_Renderer *mainRenderer, SDL_Color *colors)
{
    for (int x = 0; x < SCREEN_WIDTH; x++)
    {
        double cameraX = 2 * x / (double)SCREEN_WIDTH - 1;
        double fovRadians = 90 * M_PI / 180.0;
        double rayDirX = dirX + planeX * (cameraX * tan(fovRadians / 2));
        double rayDirY = dirY + planeY * (cameraX * tan(fovRadians / 2));

        int mapX = (int)posX;
        int mapY = (int)posY;

        double sideDistX;
        double sideDistY;

        double deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
        double deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
        double perpWallDist;

        int stepX;
        int stepY;

        int hit = 0;
        int side;

        if (rayDirX < 0)
        {
            stepX = -1;
            sideDistX = (posX - mapX) * deltaDistX;
        }
        else
        {
            stepX = 1;
            sideDistX = (mapX + 1.0 - posX) * deltaDistX;
        }
        if (rayDirY < 0)
        {
            stepY = -1;
            sideDistY = (posY - mapY) * deltaDistY;
        }
        else
        {
            stepY = 1;
            sideDistY = (mapY + 1.0 - posY) * deltaDistY;
        }

        while (hit == 0)
        {
            if (sideDistX < sideDistY)
            {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            }
            else
            {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            if (worldMap[mapX][mapY] > 0)
                hit = 1;
        }

        perpWallDist = side == 0 ? (mapX - posX + (1 - stepX) / 2) / rayDirX : (mapY - posY + (1 - stepY) / 2) / rayDirY;

        int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);

        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawStart < 0)
            drawStart = 0;
        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawEnd >= SCREEN_HEIGHT)
            drawEnd = SCREEN_HEIGHT - 1;

        SDL_Color color = colors[worldMap[mapX][mapY] - 1];

        if (side == 1)
        {
            color.r *= 0.6;
            color.g *= 0.6;
            color.b *= 0.6;
        }
        verLine(x, drawStart, drawEnd, color, mainRenderer);
    }
}

void handleInput(SDL_Event *event, double *posX, double *posY, double *dirX, double *dirY, double *planeX,
                 double *planeY, int worldMap[MAP_WIDTH][MAP_HEIGHT], double moveSpeed, double mouseSensitivity, bool *isMouseLocked,
                 bool *keys, Projectile *projectiles, bool *run, double frameTime)
{
    while (SDL_PollEvent(event))
    {
        if (event->type == SDL_QUIT)
            *run = false;
        else if (event->type == SDL_KEYDOWN)
        {
            keys[event->key.keysym.scancode] = true;
            if (event->key.keysym.scancode == SDL_SCANCODE_Q)
            {
                *isMouseLocked = !(*isMouseLocked);
                SDL_SetRelativeMouseMode(*isMouseLocked ? SDL_TRUE : SDL_FALSE);
            }
            else if (event->key.keysym.scancode == SDL_SCANCODE_SPACE)
            {
                shootProjectile(*posX, *posY, *dirX, *dirY, projectiles);
            }
        }
        else if (event->type == SDL_KEYUP)
            keys[event->key.keysym.scancode] = false;
        else if (event->type == SDL_MOUSEMOTION && *isMouseLocked)
        {
            double rotationAmountX = event->motion.xrel * mouseSensitivity;

            // Update camera direction and plane based on mouse movement
            double oldDirX = *dirX;
            *dirX = *dirX * cos(-rotationAmountX) - *dirY * sin(-rotationAmountX);
            *dirY = oldDirX * sin(-rotationAmountX) + *dirY * cos(-rotationAmountX);
            double oldPlaneX = *planeX;
            *planeX = *planeX * cos(-rotationAmountX) - *planeY * sin(-rotationAmountX);
            *planeY = oldPlaneX * sin(-rotationAmountX) + *planeY * cos(-rotationAmountX);
        }
    }

    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W])
    {
        if (worldMap[(int)(*posX + *dirX * moveSpeed)][(int)(*posY)] == 0)
            *posX += *dirX * moveSpeed;
        if (worldMap[(int)(*posX)][(int)(*posY + *dirY * moveSpeed)] == 0)
            *posY += *dirY * moveSpeed;
    }
    if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S])
    {
        if (worldMap[(int)(*posX - *dirX * moveSpeed)][(int)(*posY)] == 0)
            *posX -= *dirX * moveSpeed;
        if (worldMap[(int)(*posX)][(int)(*posY - *dirY * moveSpeed)] == 0)
            *posY -= *dirY * moveSpeed;
    }
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D])
    {
        moveSpeed = frameTime * 2.0;
        if (worldMap[(int)(*posX + *planeX * moveSpeed)][(int)*posY] == 0)
            *posX += *planeX * moveSpeed;
        if (worldMap[(int)*posX][(int)(*posY + *planeY * moveSpeed)] == 0)
            *posY += *planeY * moveSpeed;
    }

    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A])
    {
        moveSpeed = frameTime * 2.0;
        if (worldMap[(int)(*posX - *planeX * moveSpeed)][(int)*posY] == 0)
            *posX -= *planeX * moveSpeed;
        if (worldMap[(int)*posX][(int)(*posY - *planeY * moveSpeed)] == 0)
            *posY -= *planeY * moveSpeed;
    }
}
