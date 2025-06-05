#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define CELL_SIZE 20
#define WIDTH 800
#define HEIGHT 600
#define MAX_SNAKE_SIZE 100

typedef struct {
    int x, y;
} Vector2int;

Vector2int snake[MAX_SNAKE_SIZE];
int snakeLength = 5;
Vector2int direction = {1, 0};
Vector2int mouse;
bool gameOver = false;
int score = 0;

void InitGame() {
    snakeLength = 5;
    for (int i = 0; i < snakeLength; i++) {
        snake[i].x = 10 - i;
        snake[i].y = 10;
    }

    mouse.x = rand() % (WIDTH / CELL_SIZE);
    mouse.y = rand() % (HEIGHT / CELL_SIZE);
    direction = (Vector2int){1, 0};
    gameOver = false;
    score = 0;
}

void UpdateGame() {
    if (IsKeyPressed(KEY_UP) && direction.y != 1) direction = (Vector2int){0, -1};
    if (IsKeyPressed(KEY_DOWN) && direction.y != -1) direction = (Vector2int){0, 1};
    if (IsKeyPressed(KEY_LEFT) && direction.x != 1) direction = (Vector2int){-1, 0};
    if (IsKeyPressed(KEY_RIGHT) && direction.x != -1) direction = (Vector2int){1, 0};

    for (int i = snakeLength - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    // déplacement de la tête
    snake[0].x += direction.x;
    snake[0].y += direction.y;

    // collision avec les murs
    if (snake[0].x < 0 || snake[0].x >= WIDTH / CELL_SIZE ||
        snake[0].y < 0 || snake[0].y >= HEIGHT / CELL_SIZE) {
        gameOver = true;
    }

    // manger la souris
    if (snake[0].x == mouse.x && snake[0].y == mouse.y) {
        if (snakeLength < MAX_SNAKE_SIZE) snakeLength++;
        score += 10;
        mouse.x = rand() % (WIDTH / CELL_SIZE);
        mouse.y = rand() % (HEIGHT / CELL_SIZE);
    }
}

void DrawGame() {
    ClearBackground(BLACK);

    // Serpent
    for (int i = 0; i < snakeLength; i++) {
        DrawRectangle(snake[i].x * CELL_SIZE, snake[i].y * CELL_SIZE, CELL_SIZE, CELL_SIZE, GREEN);
    }

    // Souris (rouge)
    DrawRectangle(mouse.x * CELL_SIZE, mouse.y * CELL_SIZE, CELL_SIZE, CELL_SIZE, RED);

    // Score
    DrawText(TextFormat("Score: %d", score), 10, 10, 20, RAYWHITE);

    // Game Over
    if (gameOver) {
        DrawText("GAME OVER - Press R to Restart", WIDTH / 2 - 150, HEIGHT / 2, 20, WHITE);
    }
}

int main() {
    InitWindow(WIDTH, HEIGHT, "Snake Game - Raylib");
    SetTargetFPS(10);
    srand(time(NULL));

    InitGame();

    while (!WindowShouldClose()) {
        if (!gameOver) {
            UpdateGame();
        } else {
            if (IsKeyPressed(KEY_R)) {
                InitGame();
            }
        }

        BeginDrawing();
        DrawGame();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}




