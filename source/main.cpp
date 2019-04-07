#include <switch.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define QUEUE_SIZE 400
#define LEFT  1
#define UP    2
#define DOWN  3
#define RIGHT 4
#define MAX_X 39
#define MAX_Y 22
#define TILE_SIZE 32

#define BUTTON_DPAD_UP 13
#define BUTTON_DPAD_DOWN 15
#define BUTTON_DPAD_LEFT 12
#define BUTTON_DPAD_RIGHT 14
#define BUTTON_PLUS 10
#define KEY_LSTICK_UP 17
#define KEY_LSTICK_DOWN 19
#define KEY_LSTICK_LEFT 16
#define KEY_LSTICK_RIGHT 18





char dir;
char old_dir;
char eaten;
char mat[MAX_X+1][MAX_Y+1];

int delay = 16;

typedef struct tag_node {
    char x;
    char y;
} node;

node body;
node head;
node tail;
node fruit;

typedef struct tag_queue {
    int first;
    int last;
    int len;
    node elems[QUEUE_SIZE];
} queue;

queue snake;

SDL_Renderer* renderer = NULL;
SDL_Surface*  field_surface = NULL;
SDL_Surface*  fruit_surface = NULL;
SDL_Surface*  shead_surface = NULL;
SDL_Surface*  snake_surface = NULL;
SDL_Texture*  field_texture = NULL;
SDL_Texture*  fruit_texture = NULL;
SDL_Texture*  shead_texture = NULL;
SDL_Texture*  snake_texture = NULL;

void init(void);
void input(void);
void init_joycons(void);
void init_romFS(void);
int  update(void);
void render(void);
void pop_tail(void);
void push_head(void);
void gameover(void);
void draw_body(void);
void draw_head(void);
void draw_fruit(void);
void clear_tail(void);
void next_fruit(void);

int main(void)
{
    init_romFS();
    init();
    render();
    init_joycons();
    for (;;) {
        input();
        if (update()) {
            gameover();
        }
        render();
        SDL_Delay(delay * 10);
    }
    return 0;
}

void init(void)
{
    int i, j;
    SDL_Window *window = NULL;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        printf("SDL_Init: %s\n", SDL_GetError());
        return;
    }
    atexit(SDL_Quit);
    SDL_CreateWindowAndRenderer(1280, 720, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    fruit_surface = SDL_LoadBMP("romfs:/apple.bmp");
    shead_surface = SDL_LoadBMP("romfs:/head.bmp");
    snake_surface = SDL_LoadBMP("romfs:/snake.bmp");
    field_surface = SDL_LoadBMP("romfs:/field.bmp");
    fruit_texture = SDL_CreateTextureFromSurface(renderer, fruit_surface);
    shead_texture = SDL_CreateTextureFromSurface(renderer, shead_surface);
    snake_texture = SDL_CreateTextureFromSurface(renderer, snake_surface);
    field_texture = SDL_CreateTextureFromSurface(renderer, field_surface);
    for (i = 0; i <= MAX_X; i++) {
        for (j = 0; j <= MAX_Y; j++) {
            tail.x = i;
            tail.y = j;
            clear_tail();
        }
    }
    snake.first = 0;
    snake.last = 0;
    snake.len = 0;
    // srand((unsigned) (NULL));
    fruit.x = /*rand() % 16 +*/ 5;
    fruit.y = /*rand() % 6 +*/ 5;
    head = fruit;
    if (head.x < (MAX_X / 2)) {
        dir = RIGHT;
    } else {
        dir = LEFT;
    }
    push_head();
    next_fruit();
    eaten = 1;
    old_dir = 0;
    printf("Level 1\n");
}

void input(void)
{
SDL_Event event;
while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_JOYBUTTONDOWN:
                    // seek for joystick #0 down (B)
                    // https://github.com/devkitPro/SDL/blob/switch-sdl2/src/joystick/switch/SDL_sysjoystick.c#L51
                    if (event.jbutton.which == 0 && event.jbutton.button == BUTTON_DPAD_UP || event.jbutton.which == 0 && event.jbutton.button == KEY_LSTICK_UP) {
                        dir = UP;
                    } else if (event.jbutton.which == 0 && event.jbutton.button == BUTTON_DPAD_DOWN || event.jbutton.which == 0 && event.jbutton.button == KEY_LSTICK_DOWN) {
                        dir = DOWN;
                    } else if (event.jbutton.which == 0 && event.jbutton.button == BUTTON_DPAD_LEFT || event.jbutton.which == 0 && event.jbutton.button == KEY_LSTICK_LEFT) {
                        dir = LEFT;
                    } else if (event.jbutton.which == 0 && event.jbutton.button == BUTTON_DPAD_RIGHT || event.jbutton.which == 0 && event.jbutton.button == KEY_LSTICK_RIGHT) {
                        dir = RIGHT;
                    } else if (event.jbutton.which == 0 && event.jbutton.button == BUTTON_PLUS) {
                        exit(0);
                    }
                    /* Ignore opposite direction */
                    if (dir + old_dir != 5 || snake.len == 1) {
                        old_dir = dir;
                    } else {
                    dir = old_dir;
                    }
            }
}
}

void init_joycons(void) {
        for (int i = 0; i < 2; i++) {
        if (SDL_JoystickOpen(i) == NULL) {
            printf("SDL_JoystickOpen: %s\n", SDL_GetError());
            SDL_Quit();
            return;
        }
    }
}

void init_romFS(void) {
    Result rc = romfsInit();
    if (R_FAILED(rc))
        printf("romfsInit: %08X\n", rc);
}


int update(void)
{
    body = head;
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
        next_fruit();
        eaten = 1;
        switch (snake.len) {
            case 10:
                delay -= 4;
                printf("Level 2\n");
                break;
            case 20:
                delay -= 4;
                printf("Level 3\n");
                break;
            case 30:
                delay /= 2;
                printf("Level 4\n");
                break;
            case 40:
                delay /= 2;
                printf("Level 5\n");
                break;
        }
    } else {
        pop_tail();
        eaten = 0;
    }
    push_head();
    return 0;
}

void render(void)
{
    if (snake.len > 1) {
        draw_body();
    }
    if (eaten) {
        draw_fruit();
    } else {
        clear_tail();
    }
    draw_head();
    SDL_RenderPresent(renderer);
}

void pop_tail(void)
{
    tail = snake.elems[snake.first];
    snake.first = (snake.first + 1) % QUEUE_SIZE;
    snake.len--;
    mat[tail.x][tail.y] = 0;
}

void push_head(void)
{
    snake.elems[snake.last] = head;
    snake.last = (snake.last + 1) % QUEUE_SIZE;
    snake.len++;
    mat[head.x][head.y] = 1;
}

void gameover(void)
{
    printf("Snake Length: %d\n", snake.len);
    printf("Game Over\n");
    exit(0);
}

void draw_body(void)
{
    SDL_Rect rect;
    rect.h = TILE_SIZE;
    rect.w = TILE_SIZE;
    rect.x = body.x * TILE_SIZE;
    rect.y = body.y * TILE_SIZE;
    SDL_RenderCopy(renderer, snake_texture, NULL, &rect);
}

void draw_head(void)
{
    SDL_Rect rect;
    rect.h = TILE_SIZE;
    rect.w = TILE_SIZE;
    rect.x = head.x * TILE_SIZE;
    rect.y = head.y * TILE_SIZE;
    SDL_RenderCopy(renderer, shead_texture, NULL, &rect);
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
    do {
        fruit.x = (fruit.x * 6 + 1) % (MAX_X + 1);
        fruit.y = (fruit.y * 16 + 1) % (MAX_Y + 1);
    } while (mat[fruit.x][fruit.y]);
}
