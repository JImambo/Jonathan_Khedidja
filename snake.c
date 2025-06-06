#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#define MAX_SNAKE_LENGTH 100
#define MAX_OBSTACLES 20
#define HIGHSCORE_FILE "highscore.txt"
#define RECORD_FILE "record.txt"

typedef struct SnakeSegment {
    Vector2 position;
} SnakeSegment;

typedef struct Snake {
    SnakeSegment segments[MAX_SNAKE_LENGTH];
    int length;
    Vector2 direction;
    Color headColor;
    Color bodyColor;
    bool alive;
} Snake;

typedef struct Food {
    Vector2 position;
    bool isBonus;
} Food;

typedef struct Obstacle {
    Vector2 position;
} Obstacle;

const int screenWidth = 800;
const int screenHeight = 600;
const int cellSize = 20;
const int gameCols = screenWidth / cellSize;
const int gameRows = screenHeight / cellSize;

void InitSnake(Snake *snake, Vector2 startPos, Color headColor, Color bodyColor) {
    snake->length = 5;
    snake->direction = (Vector2){1, 0};
    snake->headColor = headColor;
    snake->bodyColor = bodyColor;
    snake->alive = true;
    for (int i = 0; i < snake->length; i++) {
        snake->segments[i].position = (Vector2){startPos.x - i, startPos.y};
    }
}

void MoveSnake(Snake *snake) {
    for (int i = snake->length - 1; i > 0; i--) {
        snake->segments[i].position = snake->segments[i - 1].position;
    }
    snake->segments[0].position.x += snake->direction.x;
    snake->segments[0].position.y += snake->direction.y;

    if (snake->segments[0].position.x >= gameCols) snake->segments[0].position.x = 0;
    if (snake->segments[0].position.x < 0) snake->segments[0].position.x = gameCols - 1;
    if (snake->segments[0].position.y >= gameRows) snake->segments[0].position.y = 0;
    if (snake->segments[0].position.y < 0) snake->segments[0].position.y = gameRows - 1;
}

void DrawSnake(Snake snake) {
    for (int i = 0; i < snake.length; i++) {
        DrawCircle((int)(snake.segments[i].position.x * cellSize + cellSize/2),
                   (int)(snake.segments[i].position.y * cellSize + cellSize/2),
                   cellSize/2 - 2,
                   (i == 0) ? snake.headColor : snake.bodyColor);
    }
}

bool Vector2Equals(Vector2 a, Vector2 b) {
    return (a.x == b.x && a.y == b.y);
}

bool CheckCollisionWithSelf(Snake snake) {
    for (int i = 1; i < snake.length; i++) {
        if (Vector2Equals(snake.segments[0].position, snake.segments[i].position)) {
            return true;
        }
    }
    return false;
}

bool CheckCollision(Vector2 a, Vector2 b) {
    return (a.x == b.x && a.y == b.y);
}

void SpawnFood(Food *food, Snake snakes[], int numSnakes, Obstacle obstacles[], int obstacleCount) {
    bool valid = false;
    while (!valid) {
        valid = true;
        food->position = (Vector2){GetRandomValue(0, gameCols - 1), GetRandomValue(0, gameRows - 1)};
        for (int i = 0; i < numSnakes; i++) {
            for (int j = 0; j < snakes[i].length; j++) {
                if (CheckCollision(food->position, snakes[i].segments[j].position)) valid = false;
            }
        }
        for (int i = 0; i < obstacleCount; i++) {
            if (CheckCollision(food->position, obstacles[i].position)) valid = false;
        }
    }
    food->isBonus = (GetRandomValue(0, 100) < 20);
}

void DrawFood(Food food) {
    Color color = food.isBonus ? YELLOW : RED;
    DrawCircle((int)(food.position.x * cellSize + cellSize/2),
               (int)(food.position.y * cellSize + cellSize/2),
               cellSize/2 - 2,
               color);
}

void InitObstacles(Obstacle obstacles[], int *count, Snake snakes[], int numSnakes, Food food) {
    *count = GetRandomValue(5, MAX_OBSTACLES);
    for (int i = 0; i < *count; i++) {
        bool valid = false;
        while (!valid) {
            valid = true;
            obstacles[i].position = (Vector2){GetRandomValue(0, gameCols - 1), GetRandomValue(0, gameRows - 1)};
            for (int s = 0; s < numSnakes; s++) {
                for (int j = 0; j < snakes[s].length; j++) {
                    if (CheckCollision(obstacles[i].position, snakes[s].segments[j].position)) valid = false;
                }
            }
            if (CheckCollision(obstacles[i].position, food.position)) valid = false;
        }
    }
}

void DrawObstacles(Obstacle obstacles[], int count) {
    for (int i = 0; i < count; i++) {
        DrawRectangle(obstacles[i].position.x * cellSize, obstacles[i].position.y * cellSize, cellSize, cellSize, GRAY);
    }
}

int LoadHighscore() {
    FILE *file = fopen(HIGHSCORE_FILE, "r");
    if (!file) return 0;
    int hs;
    fscanf(file, "%d", &hs);
    fclose(file);
    return hs;
}

void SaveHighscore(int score) {
    FILE *file = fopen(HIGHSCORE_FILE, "w");
    if (file) {
        fprintf(file, "%d", score);
        fclose(file);
    }
}

float LoadRecord() {
    FILE *file = fopen(RECORD_FILE, "r");
    if (!file) return 0.0f;
    float rec;
    fscanf(file, "%f", &rec);
    fclose(file);
    return rec;
}

void SaveRecord(float time) {
    FILE *file = fopen(RECORD_FILE, "w");
    if (file) {
        fprintf(file, "%f", time);
        fclose(file);
    }
}


int main() {
    InitWindow(screenWidth, screenHeight, "Snake Game");
    InitAudioDevice();

    // Charger sons et musique
    Sound eatSound = LoadSound("eat.wav");
    Sound crashSound = LoadSound("crash.wav");
    Music music = LoadMusicStream("background.mp3");
    PlayMusicStream(music);
    SetMusicVolume(music, 0.3f);

    // Variables menu
    int menuSelection = 0;    // 0=Solo,1=Duo
    int difficultySelection = 0; // 0=Easy,1=Medium,2=Hard
    bool inMenu = true;
    bool inDifficultyMenu = false;
    bool gameOver = false;
    bool pause = false;

    // Mode et difficulté définitifs
    int mode = 0;       // 0=menu,1=solo,2=duo
    int difficulty = 1; // default medium

    // Vitesse selon difficulté (FPS)
    int speeds[3] = {5, 10, 15};

    Snake snakes[2];
    Food food;
    Obstacle obstacles[MAX_OBSTACLES];
    int obstacleCount = 0;
    int score[2] = {0, 0};

    int highscore = LoadHighscore();
    float recordTime = LoadRecord();

    double startTime = 0.0;
    double elapsedTime = 0.0;

    SetTargetFPS(60); // fps constant, mais on mettra update snake selon vitesse

    while (!WindowShouldClose()) {
        UpdateMusicStream(music);

        if (inMenu) {
            BeginDrawing();
            ClearBackground(DARKGREEN);
            DrawText("SNAKE GAME", 280, 50, 40, WHITE);
            DrawText("1. Solo", 350, 150, 30, WHITE);
            DrawText("2. Duo", 350, 200, 30, WHITE);
            DrawText("Use UP/DOWN and ENTER", 260, 500, 20, WHITE);
            DrawRectangleLines(340, 150 + 50 * menuSelection, 120, 40, YELLOW);
            EndDrawing();

            if (IsKeyPressed(KEY_DOWN)) menuSelection = (menuSelection + 1) % 2;
            if (IsKeyPressed(KEY_UP)) menuSelection = (menuSelection - 1 + 2) % 2;
            if (IsKeyPressed(KEY_ENTER)) {
                inMenu = false;
                inDifficultyMenu = true;
                difficultySelection = 1; // default medium
            }
            continue;
        }

        if (inDifficultyMenu) {
            BeginDrawing();
            ClearBackground(DARKGREEN);
            DrawText("Select Difficulty", 300, 50, 30, WHITE);
            DrawText("1. Easy", 350, 150, 30, WHITE);
            DrawText("2. Medium", 350, 200, 30, WHITE);
            DrawText("3. Hard", 350, 250, 30, WHITE);
            DrawRectangleLines(340, 150 + 50 * difficultySelection, 120, 40, YELLOW);
            EndDrawing();

            if (IsKeyPressed(KEY_DOWN)) difficultySelection = (difficultySelection + 1) % 3;
            if (IsKeyPressed(KEY_UP)) difficultySelection = (difficultySelection - 1 + 3) % 3;
            if (IsKeyPressed(KEY_ENTER)) {
                difficulty = difficultySelection;
                mode = (menuSelection == 0) ? 1 : 2;
                inDifficultyMenu = false;
                gameOver = false;
                pause = false;

                // Init serpents: en duo ils sont face à face sur même ligne
                InitSnake(&snakes[0], (Vector2){5, 10}, RED, BLACK);
                snakes[0].direction = (Vector2){1, 0}; // vers droite

                if (mode == 2) {
                    InitSnake(&snakes[1], (Vector2){gameCols - 6, 10}, BLUE, DARKBLUE);
                    snakes[1].direction = (Vector2){-1, 0}; // vers gauche
                    snakes[1].alive = true;

                    printf("DEBUG - Snake 2 initialisé à (%f, %f), vivant: %d\n",
                    snakes[1].segments[0].position.x,
                    snakes[1].segments[0].position.y,
                    snakes[1].alive);
                }

                SpawnFood(&food, snakes, mode, obstacles, obstacleCount);
                InitObstacles(obstacles, &obstacleCount, snakes, mode, food);

                score[0] = 0;
                score[1] = 0;

                startTime = GetTime();

                SetTargetFPS(speeds[difficulty]);
            }
            continue;
        }

        // Gérer pause
        if (IsKeyPressed(KEY_P) && !gameOver) {
            pause = !pause;
        }

        if (!pause && !gameOver) {
            // Gérer direction serpent 1 (touches flèches)
            if (IsKeyDown(KEY_RIGHT) && snakes[0].direction.x != -1) snakes[0].direction = (Vector2){1, 0};
            else if (IsKeyDown(KEY_LEFT) && snakes[0].direction.x != 1) snakes[0].direction = (Vector2){-1, 0};
            else if (IsKeyDown(KEY_UP) && snakes[0].direction.y != 1) snakes[0].direction = (Vector2){0, -1};
            else if (IsKeyDown(KEY_DOWN) && snakes[0].direction.y != -1) snakes[0].direction = (Vector2){0, 1};

            // Gérer direction serpent 2 (mode duo: touches ZQSD)
            if (mode == 2 && snakes[1].alive) {
                if (IsKeyDown(KEY_D) && snakes[1].direction.x != -1) snakes[1].direction = (Vector2){1, 0};
                else if (IsKeyDown(KEY_A) && snakes[1].direction.x != 1) snakes[1].direction = (Vector2){-1, 0};
                else if (IsKeyDown(KEY_W) && snakes[1].direction.y != 1) snakes[1].direction = (Vector2){0, -1};
                else if (IsKeyDown(KEY_S) && snakes[1].direction.y != -1) snakes[1].direction = (Vector2){0, 1};
            }

            // Avancer les serpents selon vitesse (utiliser temps)
            static double lastMoveTime = 0;
            double currentTime = GetTime();
            if ((currentTime - lastMoveTime) > (1.0f / speeds[difficulty])) {
                lastMoveTime = currentTime;

                // Déplacer serpents
                if (snakes[0].alive) MoveSnake(&snakes[0]);
                if (mode == 2 && snakes[1].alive) MoveSnake(&snakes[1]);

                // Vérifier collisions self
                if (snakes[0].alive && CheckCollisionWithSelf(snakes[0])) {
                    snakes[0].alive = false;
                    PlaySound(crashSound);
                }
                if (mode == 2 && snakes[1].alive && CheckCollisionWithSelf(snakes[1])) {
                    snakes[1].alive = false;
                    PlaySound(crashSound);
                }

                // Vérifier collisions obstacles
                for (int i = 0; i < obstacleCount; i++) {
                    if (snakes[0].alive && CheckCollision(snakes[0].segments[0].position, obstacles[i].position)) {
                        snakes[0].alive = false;
                        PlaySound(crashSound);
                    }
                    if (mode == 2 && snakes[1].alive && CheckCollision(snakes[1].segments[0].position, obstacles[i].position)) {
                        snakes[1].alive = false;
                        PlaySound(crashSound);
                    }
                }

                // Vérifier collisions entre serpents en duo
                if (mode == 2 && snakes[0].alive && snakes[1].alive) {
                    if (CheckCollision(snakes[0].segments[0].position, snakes[1].segments[0].position)) {
                        snakes[0].alive = false;
                        snakes[1].alive = false;
                        PlaySound(crashSound);
                    }
                    // Si serpent 0 touche le corps serpent 1
                    for (int i = 0; i < snakes[1].length; i++) {
                        if (CheckCollision(snakes[0].segments[0].position, snakes[1].segments[i].position)) {
                            snakes[0].alive = false;
                            PlaySound(crashSound);
                        }
                    }
                    // Si serpent 1 touche le corps serpent 0
                    for (int i = 0; i < snakes[0].length; i++) {
                        if (CheckCollision(snakes[1].segments[0].position, snakes[0].segments[i].position)) {
                            snakes[1].alive = false;
                            PlaySound(crashSound);
                        }
                    }
                }

                // Manger nourriture
                for (int i = 0; i < mode; i++) {
                    if (snakes[i].alive && CheckCollision(snakes[i].segments[0].position, food.position)) {
                        snakes[i].length++;
                        if (snakes[i].length > MAX_SNAKE_LENGTH) snakes[i].length = MAX_SNAKE_LENGTH;
                        score[i] += food.isBonus ? 5 : 1;
                        PlaySound(eatSound);
                        SpawnFood(&food, snakes, mode, obstacles, obstacleCount);
                    }
                }

                // Fin jeu si serpent mort en solo, ou les deux morts en duo
                if (mode == 1 && !snakes[0].alive) {
                    gameOver = true;
                    elapsedTime = currentTime - startTime;
                    if (score[0] > highscore) {
                        highscore = score[0];
                        SaveHighscore(highscore);
                    }
                    if (elapsedTime > recordTime) {
                        recordTime = elapsedTime;
                        SaveRecord(recordTime);
                    }
                }
                if (mode == 2) {
                    if (!snakes[0].alive && !snakes[1].alive) {
                        gameOver = true;
                        elapsedTime = currentTime - startTime;
                        // On peut sauver highscore comme max des 2 scores
                        int maxScore = score[0] > score[1] ? score[0] : score[1];
                        if (maxScore > highscore) {
                            highscore = maxScore;
                            SaveHighscore(highscore);
                        }
                        if (elapsedTime > recordTime) {
                            recordTime = elapsedTime;
                            SaveRecord(recordTime);
                        }
                    }
                }
            }
        }

        // Dessiner tout
        BeginDrawing();
        ClearBackground((Color){85, 107, 47, 255}); // Vert militaire

        // Dessiner obstacles
        DrawObstacles(obstacles, obstacleCount);

        // Dessiner nourriture
        DrawFood(food);

        // Dessiner serpents
        if (snakes[0].alive) DrawSnake(snakes[0]);
        if (mode == 2 && snakes[1].alive) DrawSnake(snakes[1]);

        // Afficher score et temps
        DrawText(TextFormat("Score P1: %d", score[0]), 10, 10, 20, WHITE);
        if (mode == 2) DrawText(TextFormat("Score P2: %d", score[1]), 10, 40, 20, WHITE);

        double displayTime = pause ? elapsedTime : GetTime() - startTime;
        DrawText(TextFormat("Time: %.1f s", displayTime), screenWidth - 150, 10, 20, WHITE);
        DrawText(TextFormat("Highscore: %d", highscore), 10, screenHeight - 30, 20, YELLOW);
        DrawText(TextFormat("Record: %.1f s", recordTime), screenWidth - 180, screenHeight - 30, 20, YELLOW);

        if (pause) {
            DrawText("PAUSED", screenWidth/2 - 50, screenHeight/2, 40, ORANGE);
        }

        if (gameOver) {
            DrawRectangle(200, 200, 400, 200, Fade(BLACK, 0.7f));
            DrawText("GAME OVER", 320, 220, 40, RED);

            if (mode == 1) {
                DrawText(TextFormat("Your score: %d", score[0]), 320, 270, 30, WHITE);
            } else if (mode == 2) {
                if (snakes[0].alive && !snakes[1].alive) {
                    DrawText("Player 1 Wins!", 320, 270, 30, GREEN);
                } else if (!snakes[0].alive && snakes[1].alive) {
                    DrawText("Player 2 Wins!", 320, 270, 30, GREEN);
                } else {
                    DrawText("It's a tie!", 320, 270, 30, WHITE);
                }
                DrawText(TextFormat("Scores P1: %d  P2: %d", score[0], score[1]), 280, 310, 20, WHITE);
            }

            DrawText("Press R to Restart", 300, 360, 20, WHITE);
            DrawText("Press M for Menu", 300, 390, 20, WHITE);

            if (IsKeyPressed(KEY_R)) {
                inDifficultyMenu = true;
                gameOver = false;
                pause = false;
            }
            if (IsKeyPressed(KEY_M)) {
                inMenu = true;
                gameOver = false;
                pause = false;
            }
        }

        EndDrawing();
    }

    // Cleanup
    UnloadSound(eatSound);
    UnloadSound(crashSound);
    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}