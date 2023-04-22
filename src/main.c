// INCLUDE FILES
#include "../include/main.h"
#include <SDL2/SDL.h>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// TYPEDEF FOR BOOLEANS
typedef int bool_t;

#define B_TRUE  1
#define B_FALSE 0

// WINDOW DEFINITION
struct _window {
    char* title;
    int width, height;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    const uint8_t* keys;

    SDL_Event event;

    uint32_t* pixels;
};

typedef struct _window window;

// CREATE WINDOW
window create_window(char* title, int width, int height) {
    window win = {0};

    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        perror("[ERROR] Failed to init SDL\n");
        exit(EXIT_FAILURE);
    }

    printf("[INFO]  Initialized SDL\n");

    win.title = title;
    win.width = width;
    win.height = height;

    win.window = SDL_CreateWindow(title, 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        width, height, 0
    );

    if(!win.window) {
        perror("[ERROR] Failed to init window\n");
        exit(EXIT_FAILURE);
    }

    printf("[INFO]  Initialized Window\n");

    win.renderer = SDL_CreateRenderer(win.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if(!win.renderer) {
        perror("[ERROR] Failed to init renderer\n");
        exit(EXIT_FAILURE);
    }

    printf("[INFO]  Initialized Renderer\n");

    win.pixels = (uint32_t*)malloc(width*height*sizeof(uint32_t));
    win.texture = SDL_CreateTexture(
        win.renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        width, height
    );

    SDL_PumpEvents();
    win.keys = SDL_GetKeyboardState(NULL);

    return win;
}

// CHECK IF WINDOW IS RUNNING
bool_t window_running(window* win) {
    while(SDL_PollEvent(&win->event)) {
        switch(win->event.type) {
            case SDL_QUIT:
                return B_FALSE;
        }
    }

    win->keys = SDL_GetKeyboardState(NULL);

    return B_TRUE;
}

// CLEAR WINDOW BUFFER
void window_clear(window* win, uint32_t col) {
    for(int i = 0; i < win->width * win->height; ++i) {
        win->pixels[i] = col;
    }
}

// RENDER BUFFER TO WINDOW
void window_render(window* win) {
    SDL_UpdateTexture(win->texture, NULL, win->pixels, win->width*sizeof(uint32_t));
    SDL_RenderCopy(win->renderer, win->texture, NULL, NULL);

    SDL_RenderPresent(win->renderer);
}

// DESTROY WINDOW
void window_destroy(window* win) {
    free(win->pixels);
    printf("[INFO]  Freed pixel array memory\n");

    SDL_DestroyTexture(win->texture);
    printf("[INFO]  Destroyed Render Texture\n");

    SDL_DestroyRenderer(win->renderer);
    printf("[INFO]  Destroyed Renderer\n");

    SDL_DestroyWindow(win->window);
    printf("[INFO]  Destroyed Window\n");

    SDL_Quit();
    printf("[INFO]  Terminated SDL\n");
}

// SET SINGLE PIXEL
void window_set_pixel(window* win, int x, int y, uint32_t col) {
    if(y >= win->height || x >= win->width) {
        return;
    } if (y < 0 || x < 0) {
        return;
    }

    uint32_t index = y * win->width + x;

    win->pixels[index] = col;
}

void window_draw_vertical_line(window* win, int x, int y1, int y2, uint32_t col) {
    if(y1 > y2) {

        for(int y = y2; y < y1; ++y) {
            window_set_pixel(win, x, y, col);
        }

    } else {

        for(int y = y1; y < y2; ++y) {
            window_set_pixel(win, x, y, col);
        }

    }
}

// FILL RECT WITH COLOUR
void window_fill_area(window* win, int x1, int y1, int x2, int y2, uint32_t col) {
    for(int y = y1; y < y2 + y1; ++y) {

        if(y >= win->height) continue;

        for(int x = x1; x < x2 + x1; ++x) {
            if(x >= win->width) continue;

            window_set_pixel(win, x, y, col);
        }
    }
}

float px = 5, py = 5;
float pa = 0;
float fov = 60;

static uint32_t* cols;

static int MAP_W, MAP_H;
static int* map;

bool_t is_digit(char c) {
    return (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || 
            c == '5' || c == '6' || c == '7' || c == '8' || c == '9');
}

int get_first_int(char* buffer, int start, char** res_buf) {
    int index = start;
    
    while(buffer[index] != '\0') {
        if(is_digit(buffer[index])) {
            char* ptr = buffer + index;
            int res = strtol(ptr, &ptr, 10);
            *res_buf = ptr;

            return res;
        }

        index++;
    }

    return -1;
}

void load_map(char* path) {
    FILE* file = fopen(path, "r");
    printf("[INFO]  Loading file %s\n", path);

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    printf("[INFO]  Reading file contents\n");
    char* buffer = (char*)malloc(size + 1);
    fread(buffer, 1, size, file);
    buffer[size] = '\0';

    printf("[INFO]  Loading player data\n");
    px = get_first_int(buffer, 0, &buffer);
    py = get_first_int(buffer, 0, &buffer);

    printf("[INFO]  Loading colour data\n");
    int num_cols = get_first_int(buffer, 0, &buffer);

    cols = (uint32_t*)malloc(num_cols * sizeof(uint32_t));

    for(int i = 0; i < num_cols; ++i) {
        uint32_t r = get_first_int(buffer, 0, &buffer);
        uint32_t g = get_first_int(buffer, 0, &buffer);
        uint32_t b = get_first_int(buffer, 0, &buffer);

        cols[i] = (r << 24) + (g << 16) + (b << 8) + 255;
        // printf("%x\n", cols[i]);
    }

    printf("[INFO]  Loading map data\n");
    int map_w = get_first_int(buffer, 0, &buffer);
    int map_h = get_first_int(buffer, 0, &buffer);

    map = (int*)malloc(map_w * map_h * sizeof(int));

    MAP_W = map_w;
    MAP_H = map_h;

    for(int i = 0; i < MAP_W * MAP_H; ++i) {
        map[i] = get_first_int(buffer, 0, &buffer);
    }

    printf("[INFO]  Map loading complete\n");
    fclose(file);
}

float deg2rad(float deg) {
    return deg * (3.141593 / 180); 
}

float rad2deg(float rad) {
    return rad / (3.141593 / 180);
}

// Renders the raycast
void render_raycast(window* win) {
    // Fill floor and ceiling
    window_fill_area(win, 0, 0, win->width, win->height / 2, 0x646464FF);
    window_fill_area(win, 0, win->height / 2, win->width, win->height, 0x323232FF);

    // Starting ray angle
    float ra = pa - (fov / 2);

    // Iterate through each pixel column
    for(int r = 0; r < win->width; ++r) {
        // Get the cosine and sine of the angle
        float ca = cos(deg2rad(ra)) / 64;
        float sa = sin(deg2rad(ra)) / 64;

        // Set the ray positions
        float rx = px, ry = py;

        // Will take the value of each tile the ray travels through
        int wall = 0;

        // While the tile is a blank space
        while(wall == 0) {
            // Increment the positions
            rx += ca;
            ry += sa;

            // Set the wall value
            wall = map[(int)(floor(ry) * MAP_W + floor(rx))];
        }

        // Calculate the distance
        float dist = sqrtf(powf(px - rx, 2) + powf(py - ry, 2));

        // Fix fisheye effect
        dist *= cosf(deg2rad(ra - pa));

        // Calculate the height of the wall (perspective division)
        float height = floorf(win->height / dist) / 2;
       
        // Clamp the wall height to prevent illegal memory writing
        if(height < 0) height = 0;
        if(height >= win->height / 2) height = (win->height / 2) - 1;

        // Get the ray colour
        uint32_t col = cols[wall - 1];
      
        // Draw the pixel column
        window_draw_vertical_line(win, r, (int)(win->height/2) - height, (int)(win->height/2), col);
        window_draw_vertical_line(win, r, (int)(win->height/2), (int)(win->height/2) + height, col);

        // Increment the ray angle
        ra += fov / (win->width);
    }
}

bool_t can_move_y(float ny) {
    int wall = map[(int)floorf(ny) * MAP_W + (int)floorf(px)];

    if(wall == 1) {
        map[(int)floorf(ny) * MAP_W + (int)floorf(px)] = 0;
    }

    return wall == 0 || wall == 1;
}

bool_t can_move_x(float nx) {
    int wall = map[(int)floorf(py) * MAP_W + (int)floorf(nx)];

    if(wall == 1) {
        map[(int)floorf(py) * MAP_W + (int)floorf(nx)] = 0;
    }

    return wall == 0 || wall == 1;
}

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: exec <map_file_path>\n");
        return -1;
    }

    load_map(argv[1]);

    window win = create_window("Test Window", 1600, 900);

    render_raycast(&win);

    while(window_running(&win)) {
        window_clear(&win, 0x000000FF);

        render_raycast(&win);
        // pa += 0.1f;

        if(win.keys[SDL_SCANCODE_LEFT]) pa -= 1.5f;
        if(win.keys[SDL_SCANCODE_RIGHT]) pa += 1.5f;

        float c = cosf(deg2rad(pa)) * 0.0625;
        float s = sinf(deg2rad(pa)) * 0.0625;

        float nx = px;
        float ny = py;

        if(win.keys[SDL_SCANCODE_W]) {
            nx += c;
            ny += s;
        }

        if(win.keys[SDL_SCANCODE_S]) {
            nx -= c;
            ny -= s;
        }

        if(win.keys[SDL_SCANCODE_A]) {
            nx += s;
            ny -= c;
        }

        if(win.keys[SDL_SCANCODE_D]) {
            nx -= s;
            ny += c;
        }

        if(can_move_y(ny)) py = ny; 
        if(can_move_x(nx)) px = nx;

        window_render(&win);
    }

    window_destroy(&win);
	return 0;
}
