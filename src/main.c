
/** main.c, maze-generator-c
 *
 *  Copyright (C) 2021 Czespo
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "blist.h"

#include <SDL.h>

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

// P R O T O T Y P E S //
// =================== //

bool init();
void quit();

void input();

bool out_of_bounds(int x, int y);
bool can_move(int x, int y);

void update();
void render();

bool saveBMP();

// P O I N T //
// ========= //

typedef struct point_s {
    int x, y;
} point_t;

// P A T H S //
// ========= //

typedef struct paths_s {
    int _[5];
} paths_t;

paths_t get_paths(point_t* point);
int count_paths(point_t* point);

// H E A D //
// ======= //

typedef struct head_s {
    point_t point;
    int direction;

    blist_t branches;
} head_t;

// G L O B A L S //
// ============= //

enum {
    MODE_RANDOM_SWITCHING = 1,
    MODE_DEPTH_FIRST = 2,
    MODE_BREADTH_FIRST = 3,

    MOVE_UP = 1,
    MOVE_RIGHT = 2,
    MOVE_DOWN = 3,
    MOVE_LEFT = 4,
};

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

static int window_width = 800, window_height = 640;

static int delay = 100; // 10 FPS.

static int step = 2;
static int numHeads = 1;

static int maze_width = 20, maze_height = 20;

static int switch_chance = 10;

static int mode = MODE_DEPTH_FIRST;

static blist_t* heads = NULL;

static char* map = NULL;
static int maze_width, maze_height;

static int cellSize, xp, yp;

static bool quiet = false;
static bool running = true;

static const char* default_outfile = "maze.bmp";
static const char* outfile = NULL;

// M A I N //
// ======= //

int main(int argc, char** argv)
{
    if(argc == 2 && strcmp(argv[1], "-h") == 0)
    {
        printf("usage: mazegen [options]\n");
        printf("  -h                        Shows this help message.\n");
        printf("  -w <width> <height>       Width and height of the viewer window. Default 800x640.\n");
        printf("  -f -fps <number>          FPS of the viewer. Default 10.\n");
        printf("  -s -size <width> <height> Width and height of the maze. Default 20x20.\n");
        printf("  -m -mode <name>           Method used to generate the maze. One of 'random', 'depth', 'breadth'. Default 'depth'.\n");
        printf("  -switch <chance>          Chance (0-100) that a head will switch to another branch. Default 10.\n");
        printf("  -step <size>              Number of steps the head takes in any direction. Default 2.\n");
        printf("  -h -heads <number>        Number of heads that create the maze. Default 1.\n");
        printf("  -q                        A window wont be created which shows the maze generation.\n");
        printf("  -seed <seed>              Seed used for random number generation. Default RANDOM.\n");
        printf("  -o <path>                 Saves the final state of the maze to this file. Default 'maze.bmp'.\n");
        return EXIT_SUCCESS;
    }

    if(!init()) return 1;

    outfile = default_outfile;
    long seed = 0;

    // Parse command-line arguments. //
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-w") == 0)
        {
            window_width = atoi(argv[++i]);
            window_height = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "-size") == 0)
        {
            maze_width = atoi(argv[++i]);
            maze_height = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "-mode") == 0)
        {
            if(strcmp(argv[i + 1], "random") == 0)
            {
                mode = MODE_RANDOM_SWITCHING;
            }
            else if(strcmp(argv[i + 1], "depth") == 0)
            {
                mode = MODE_DEPTH_FIRST;
            }
            else if(strcmp(argv[i + 1], "breadth") == 0)
            {
                mode = MODE_BREADTH_FIRST;
            }
        }
        else if(strcmp(argv[i], "-step") == 0)
        {
            step = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-switch") == 0)
        {
            switch_chance = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-heads") == 0)
        {
            numHeads = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "-fps") == 0)
        {
            int fps = atoi(argv[++i]);
            if(fps)
            {
                delay = 1000 / fps;
            }
            else
            {
                delay = 0;
            }
        }
        else if(strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "-quiet") == 0)
        {
            quiet = true;
            delay = 0;
        }
        else if(strcmp(argv[i], "-seed") == 0)
        {
            seed = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-o") == 0)
        {
            outfile = argv[++i];
        }
    }

    if(maze_width == 1 || maze_height == 1)
    {
        fprintf(stderr, "error: both maze dimensions must be greater than 1.\n");
        return EXIT_SUCCESS;
    }

    // Set maze width and height. //
    maze_width = maze_width * step - 1;
    maze_height = maze_height * step - 1;

    // Seed prng. //
    if(seed == 0) seed = time(NULL);
    printf("Running with seed: %ld\n", seed);
    srand(seed);

    if(!quiet)
    {
        // Initialise window and renderer. //
        window = SDL_CreateWindow("mazegen", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_SHOWN);
        if(window == NULL)
        {
            fprintf(stderr, "Window could not be created! SDL Error: %s\n", SDL_GetError());
            quit();
            return EXIT_FAILURE;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if(renderer == NULL)
        {
            fprintf(stderr, "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
            quit();
            return EXIT_FAILURE;
        }
    }

    // Initialise cellSize and padding.
    cellSize = fmin(window_width / maze_width, window_height / maze_height);
    xp = (window_width - (cellSize * maze_width)) / 2;
    yp = (window_height - (cellSize * maze_height)) / 2;

    // Initialise maze map. //
    map = calloc(maze_width * maze_height, 1);
    if(map == NULL)
    {
        fprintf(stderr, "Failed to allocate maze map!\n");
        quit();
        return EXIT_FAILURE;
    }

    // Initialise heads. //
    heads = blist_create(numHeads, sizeof(head_t));
    if(heads == NULL)
    {
        fprintf(stderr, "Failed to allocate heads list!\n");
        quit();
        return EXIT_FAILURE;
    }

    for(int i = 0; i < numHeads; i++)
    {
        head_t head = {{rand() % (maze_width / step) * step, rand() % (maze_height / step) * step}, rand() % 4, {0}};
        if(!blist_init(&head.branches, 10, sizeof(point_t)))
        {
            fprintf(stderr, "Failed to allocate head branches list!\n");
            quit();
            return EXIT_FAILURE;
        }

        blist_push(heads, &head);

        map[head.point.y * maze_width + head.point.x] = 1;
    }

    while(heads->length && running)
    {
        clock_t start = clock();

        input();
        update();

        if(!quiet) render();

        if(delay)
        {
            int dl = delay - (clock() - start);
            if(dl > 0) SDL_Delay(dl);
        }
    }

    if(saveBMP()) printf("Saved maze to '%s'!\n", outfile);

    if(!quiet)
    {
        // Loop until the user quits.
        while(running)
        {
            input();
            SDL_Delay(20);
        }
    }

    quit();
    return EXIT_SUCCESS;
}

// F U N C T I O N S //
// ================= //

bool init()
{
    // Initialise SDL. //
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL could not initialise! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

void quit()
{
    if(heads != NULL)
        blist_destroy(heads);

    if(map != NULL)
        free(map);

    // Close SDL. //
    if(SDL_WasInit(0) > 0)
    {
        if(window != NULL)
            SDL_DestroyWindow(window);

        if(renderer != NULL)
            SDL_DestroyRenderer(renderer);

        SDL_Quit();
    }
}

void input()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN:

                if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    running = false;
                break;

            case SDL_KEYUP:

                if(event.key.keysym.scancode == SDL_SCANCODE_MINUS)
                {
                    delay += 20;
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_EQUALS)
                {
                    if(delay >= 20) delay -= 20;
                }
        }
    }
}

bool out_of_bounds(int x, int y)
{
    if(x < 0 || x >= maze_width || y < 0 || y >= maze_height)
        return true;

    return false;
}

bool can_move(int x, int y)
{
    if(!out_of_bounds(x, y) && map[y * maze_width + x] == 0)
        return true;

    return false;
}

paths_t get_paths(point_t* point)
{
    paths_t paths = {0};
    int index = 1;

    if(can_move(point->x, point->y - step))
        paths._[index++] = MOVE_UP;

    if(can_move(point->x + step, point->y))
        paths._[index++] = MOVE_RIGHT;

    if(can_move(point->x, point->y + step))
        paths._[index++] = MOVE_DOWN;

    if(can_move(point->x - step, point->y))
        paths._[index++] = MOVE_LEFT;

    paths._[0] = index - 1;

    return paths;
}

int count_paths(point_t* point)
{
    return get_paths(point)._[0];
}

void head_push_branch(head_t* head, point_t* point)
{
    point_t new_branch = {point->x, point->y};
    blist_push(&head->branches, &new_branch);
}

void head_switch_branch(head_t* head)
{
    point_t branch;
    if(mode == MODE_RANDOM_SWITCHING)
    {
        // Select a random branch from branches.
        size_t index = rand() % head->branches.length;
        blist_copyget(&head->branches, index, &branch);
        blist_remove(&head->branches, index);
    }
    else if(mode == MODE_DEPTH_FIRST)
    {
        // Select branch at top of branches.
        blist_copyget(&head->branches, head->branches.length - 1, &branch);
        blist_pop(&head->branches);
    }
    else if(mode == MODE_BREADTH_FIRST)
    {
        // Select branch at bottom of branches.
        blist_copyget(&head->branches, 0, &branch);
        blist_remove(&head->branches, 0);
    }

    // Push head to branches if head has any paths.
    if(count_paths(&head->point)) head_push_branch(head, &head->point);

    // Set head to this branch.
    head->point.x = branch.x;
    head->point.y = branch.y;
}

void update()
{
    bool removed = false;
    for(size_t i = 0; i < heads->length; i++)
    {
        head_t* head = blist_get(heads, i);

        // In MODE_RANDOM_SWITCHING, switch to a new branch with switch_chance probability.
        if(mode == MODE_RANDOM_SWITCHING && head->branches.length > 0 && rand() % 100 + 1 <= switch_chance)
            head_switch_branch(head);

        // Switch branches until the head has paths or no branches remain.
        while(count_paths(&head->point) == 0)
        {
            if(head->branches.length == 0)
            {
                // No paths and no branches, remove this head.
                free(head->branches.array);
                blist_remove(heads, i--);
                removed = true;
                break;
            }

            head_switch_branch(head);
        }

        if(removed)
        {
            removed = false;
            continue;
        }

        // Move the head in a random available direction.
        paths_t paths = get_paths(&head->point);

        int direction = paths._[rand() % paths._[0] + 1];

        point_t old_head = {head->point.x, head->point.y};

        for(int i = 0; i < step; i++)
        {
            if(direction == MOVE_UP)
            {
                head->point.y--;
            }
            else if(direction == MOVE_RIGHT)
            {
                head->point.x++;
            }
            else if(direction == MOVE_DOWN)
            {
                head->point.y++;
            }
            else if(direction == MOVE_LEFT)
            {
                head->point.x--;
            }

            map[head->point.y * maze_width + head->point.x] = 1;
        }

        // Push a branch if the old head had any paths.
        if(count_paths(&old_head)) head_push_branch(head, &old_head);

        head->direction = direction;
    }
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    // Render map. //
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    for(int y = 0; y < maze_height; y++)
    {
        for(int x = 0; x < maze_width; x++)
        {
            if(map[y * maze_width + x] == 1)
            {
                SDL_Rect cell = {x * cellSize + xp, y * cellSize + yp, cellSize, cellSize};
                SDL_RenderFillRect(renderer, &cell);
            }
        }
    }

    // Render heads. //
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x40, 0x40, 0xFF);
    for(size_t i = 0; i < heads->length; i++)
    {
        head_t* head = blist_get(heads, i);

        SDL_Rect headr = {head->point.x * cellSize + xp, head->point.y * cellSize + yp, cellSize, cellSize};
        SDL_RenderFillRect(renderer, &headr);
    }

    SDL_RenderPresent(renderer);
}

// U T I L I T I E S //
// ================= //

bool saveBMP()
{
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, maze_width + 2, maze_height + 2, 24, SDL_PIXELFORMAT_RGB24);
    if(surface == NULL) return false;

    for(int y = 0; y < maze_height; y++)
    {
        for(int x = 0; x < maze_width; x++)
        {
            if(map[y * maze_width + x] == 1)
            {
                SDL_Rect r = {x + 1, y + 1, 1, 1};
                SDL_FillRect(surface, &r, 0xFFFFFF);
            }
        }
    }

    if(SDL_SaveBMP(surface, outfile) != 0)
    {
        fprintf(stderr, "saveBMP() Failed to save bmp. SDL Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return false;
    }

    SDL_FreeSurface(surface);
    return true;
}
