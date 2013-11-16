#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>

#define QUEUE_SIZE 400
#define LEFT  1
#define UP    2
#define DOWN  3
#define RIGHT 4
#define MAX_X 24
#define MAX_Y 14
#define TILE_SIZE 32

int dir;
int old_dir;
char mat[MAX_X+1][MAX_Y+1];

typedef struct tag_node {
	int x;
	int y;
} node;

node head;
node tail;
node fruit;

typedef struct tag_queue {
	int start;
	int end;
	node elems[QUEUE_SIZE];
} queue;

queue snake;

SDL_Renderer* renderer = NULL;
SDL_Texture*  field_texture = NULL;
SDL_Texture*  fruit_texture = NULL;
SDL_Texture*  snake_texture = NULL;

void init(void);
void input(void);
int  update(void);
void render(void);
void pop_tail(void);
void push_head(void);
void gameover(void);
void draw_head(void);
void draw_fruit(void);
void clear_tail(void);
void next_fruit(void);

int main(void)
{
    int i, j;
    int gameover = 0;
    SDL_Window *window = NULL;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    }
    if (IMG_Init(0) != 0) {
        fprintf(stderr, "IMG_Init: %s\n", SDL_GetError());
    }
    atexit(SDL_Quit);
    atexit(IMG_Quit);
    SDL_CreateWindowAndRenderer(800, 480, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    fruit_texture = IMG_LoadTexture(renderer, "apple.png");
    snake_texture = IMG_LoadTexture(renderer, "snake.png");
    field_texture = IMG_LoadTexture(renderer, "field.jpg");
    for (i = 0; i <= MAX_X; i++) {
        for (j = 0; j <= MAX_Y; j++) {
            tail.x = i;
            tail.y = j;
            clear_tail();
        }
    }
	init();
	render();
	while (1) {
		input();
		if (!gameover) {
			gameover = update();
		}
		render();
        SDL_Delay(100);
    }
	return 0;
}

void init(void)
{
	snake.start = 0;
	snake.end = 0;
    srand((unsigned)time(NULL));
    fruit.x = rand() % 16 + 5;
    fruit.y = rand() % 6 + 5;
    head = fruit;
    if (head.x < (MAX_X / 2)) {
        dir = RIGHT;
    } else {
        dir = LEFT;
    }
	push_head();
	mat[head.x][head.y] = 1;
	next_fruit();
    draw_fruit();
}

void input(void)
{
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    SDL_PumpEvents();
	if (state[SDL_SCANCODE_UP]) {
        dir = UP;
    } else if (state[SDL_SCANCODE_DOWN]) {
        dir = DOWN;
    } else if (state[SDL_SCANCODE_LEFT]) {
        dir = LEFT;
    } else if (state[SDL_SCANCODE_RIGHT]) {
        dir = RIGHT;
    } else if (state[SDL_SCANCODE_ESCAPE]) {
        exit(0);
    }
    /* Ignore opposite direction */
    if (dir + old_dir != 5) {
        old_dir = dir;
    } else {
        dir = old_dir;
    }
}

int update(void)
{
    SDL_Rect rect;
    rect.h = TILE_SIZE;
    rect.w = TILE_SIZE;
	switch (dir) {
        case UP:
            head.y = head.y - 1;
            break;
        case DOWN:
            head.y = head.y + 1;
            break;
        case LEFT:
            head.x = head.x - 1;
            break;
        case RIGHT:
            head.x = head.x + 1;
            break;
	}
	if (head.x < 0 || head.x > MAX_X || head.y < 0 || head.y > MAX_Y) {
		return 1;
	}
	if (mat[head.x][head.y]) {
		return 1;
	}
	if (head.x == fruit.x && head.y == fruit.y) {
        do {
            next_fruit();
        } while (mat[fruit.x][fruit.y]);
        draw_fruit();
	} else {
		pop_tail();
		mat[tail.x][tail.y] = 0;
        clear_tail();
	}
	mat[head.x][head.y] = 1;
	push_head();
    draw_head();
	return 0;
}

void render(void)
{
    SDL_RenderPresent(renderer);
}

void pop_tail(void)
{
	tail = snake.elems[snake.start];
	snake.start = (snake.start + 1) % QUEUE_SIZE;
}

void push_head(void)
{
	snake.elems[snake.end] = head;
	snake.end = (snake.end + 1) % QUEUE_SIZE;
}

void draw_head(void)
{
    SDL_Rect rect;
    rect.h = TILE_SIZE;
    rect.w = TILE_SIZE;
    rect.x = head.x * TILE_SIZE;
    rect.y = head.y * TILE_SIZE;
    SDL_RenderCopy(renderer, snake_texture, NULL, &rect);
}

void draw_fruit(void)
{
    SDL_Rect rect;
    rect.h = TILE_SIZE;
    rect.w = TILE_SIZE;
    rect.x = fruit.x * TILE_SIZE;
    rect.y = fruit.y * TILE_SIZE;
    SDL_RenderCopy(renderer, fruit_texture, NULL, &rect);
}

void clear_tail(void)
{
    SDL_Rect rect;
    rect.h = TILE_SIZE;
    rect.w = TILE_SIZE;
    rect.x = tail.x * TILE_SIZE;
    rect.y = tail.y * TILE_SIZE;
    SDL_RenderCopy(renderer, field_texture, NULL, &rect);
    /* SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
     SDL_RenderFillRect(renderer, &rect); */
}

void next_fruit(void)
{
    fruit.x = (fruit.x * 6 + 1) % (MAX_X + 1);
    fruit.y = (fruit.y * 16 + 1) % (MAX_Y + 1);
}
