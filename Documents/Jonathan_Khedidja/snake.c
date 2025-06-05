#include "raylib.h"
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>



#define SQUARE_SIZE 20
#define MAX_SNAKE_LENGTH 256

typedef struct SnakeSegment {
    int x, y;
} SnakeSegment;

typedef struct Food {
    Vector2 pos;
    bool isInsect; // true = insect, false = normal food
} Food;

// Génère une nouvelle nourriture aléatoire
Food GenerateFood(int screenWidth, int screenHeight) {
    Food f;
    f.pos.x = GetRandomValue(0, screenWidth / SQUARE_SIZE - 1);
    f.pos.y = GetRandomValue(0, screenHeight / SQUARE_SIZE -1);
    f.isInsect = (GetRandomValue(0, 4) == 0);
    return f;
}

bool CheckSelfCollision(SnakeSegment *snake, int length) {
    for (int i = 1; i < length; i++) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y){
           return true;
        }
    }
    return false;
}


int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Jonathan_Khedidja - Snake - Raylib");
    SetTargetFPS(10);
    InitAudioDevice();
    srand(time(NULL));

    bool playAgain = true;


   while (playAgain && !WindowShouldClose()) {
      // Initialisation du jeu

          SnakeSegment snake[MAX_SNAKE_LENGTH];
          int length = 1;
          int score = 0;
          snake[0].x = 10;
          snake[0].y = 10;

          int dx = 1, dy = 0;

          Food food = GenerateFood(screenWidth, screenHeight);
          bool gameOver = false;


          while (!gameOver && !WindowShouldClose()) {
              // Input
                if (IsKeyPressed(KEY_RIGHT) && dx == 0) { dx = 1; dy = 0; }
                if (IsKeyPressed(KEY_LEFT) && dx == 0)  { dx = -1; dy = 0; }
                if (IsKeyPressed(KEY_UP) && dy == 0)    { dx = 0; dy = -1; }
                if (IsKeyPressed(KEY_DOWN) && dy == 0)  { dx = 0; dy = 1; }

              // Déplacement du snake
                for (int i = length - 1; i > 0; i--) {
                    snake[i] = snake[i - 1];
                }

              // Move head
                snake[0].x += dx;
                snake[0].y += dy;

              // Wrap around
                if (snake[0].x < 0) snake[0].x = screenWidth / SQUARE_SIZE - 1;
                if (snake[0].x >= screenWidth / SQUARE_SIZE) snake[0].x = 0;
                if (snake[0].y < 0) snake[0].y = screenHeight / SQUARE_SIZE - 1;
                if (snake [0].y >= screenHeight / SQUARE_SIZE) snake[0].y = 0;


              // Collision avec le food
                if (snake[0].x == (int)food.pos.x && snake[0].y == (int)food.pos.y) {
                    if (length < MAX_SNAKE_LENGTH) length++;
                    score += food.isInsect ? 5 : 1;
                    food = GenerateFood(screenWidth, screenHeight);
                }

              // Self collision
                if (CheckSelfCollision(snake, length)) {
                    gameOver = true;
                }

              // Affichage
                BeginDrawing();
                ClearBackground(BLACK);

              // Affichage du score en haut à gauche
                DrawText(TextFormat("Score: %d", score), 10, 10, 20, WHITE);

              // Draw snake
                for (int i = 0; i < length; i++) {
                    DrawRectangle(snake[i].x * SQUARE_SIZE, snake[i].y * SQUARE_SIZE,
                             SQUARE_SIZE, SQUARE_SIZE, i == 0 ? YELLOW : GREEN);
                }

              // Draw food
                Color foodColor = food.isInsect ? ORANGE : RED;
                DrawRectangle(food.pos.x * SQUARE_SIZE, food.pos.y * SQUARE_SIZE,
                       SQUARE_SIZE, SQUARE_SIZE, foodColor);
         
                EndDrawing();
        }

        // Affiche menu de fin
        while (!WindowShouldClose()) {
             BeginDrawing();
             ClearBackground(BLACK);
            

             DrawText("GAME OVER!", screenWidth / 2 - 100, screenHeight / 2 - 60, 40, RED);
             DrawText(TextFormat("Final score: %d", score), screenWidth / 2 - 80, screenHeight / 2 - 10, 20, WHITE);
             DrawText("Press [R] to Restart or [ESC] to Quit", screenWidth / 2 - 160, screenHeight / 2 + 30, 20, GRAY);

             EndDrawing();

             if (IsKeyPressed(KEY_R)) {
                playAgain = true;
                break;
             }
        }
   }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

