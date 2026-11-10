#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ppu-types.h>
#include <sys/process.h>
#include <io/pad.h>

#define MAP_WIDTH 1920
#define MAP_HEIGHT 1080
#define MAX_SNAKE_LENGTH 100

typedef struct {
    float x, y;
} Point;

typedef struct {
    Point body[MAX_SNAKE_LENGTH];
    int length;
    float speed;
    int score;
    int isAlive;
} Snake;

void initGame(Snake *player, Snake *enemy) {
    player->body[0].x = MAP_WIDTH / 2.0f;
    player->body[0].y = MAP_HEIGHT / 2.0f;
    player->length = 5;
    player->speed = 4.0f;
    player->score = 0;
    player->isAlive = 1;

    enemy->body[0].x = 300.0f;
    enemy->body[0].y = 300.0f;
    enemy->length = 5;
    enemy->speed = 3.0f;
    enemy->score = 0;
    enemy->isAlive = 1;
}

void updateAI(Snake *enemy, Point playerPos) {
    if (!enemy->isAlive) return;
    if (enemy->body[0].x < playerPos.x) enemy->body[0].x += enemy->speed;
    if (enemy->body[0].x > playerPos.x) enemy->body[0].x -= enemy->speed;
    if (enemy->body[0].y < playerPos.y) enemy->body[0].y += enemy->speed;
    if (enemy->body[0].y > playerPos.y) enemy->body[0].y -= enemy->speed;
}

int main(int argc, char *argv[]) {
    printf("=========================================\n");
    printf("    SLITHER.IO OFFLINE - PS3 HOMEBREW    \n");
    printf("=========================================\n");

    ioPadInit(7);
    padData pad;

    Snake player, enemy;
    initGame(&player, &enemy);

    int emMenu = 1;
    printf("[MENU] Pressione START para iniciar o jogo...\n");

    while (1) {
        ioPadGetState(0, &pad);

        if (emMenu) {
            if (pad.button[2] & PAD_START) {
                emMenu = 0;
                printf("[JOGO] A iniciar Slither.io Offline...\n");
            }
            sysTimerSleep(1);
            continue;
        }

        // Sistema de Boost com o botão X (Cruz)
        if (pad.button[2] & PAD_CROSS) {
            player.speed = 7.0f;
        } else {
            player.speed = 4.0f;
        }

        // Controlos do D-Pad
        if (pad.button[2] & PAD_UP)    player.body[0].y -= player.speed;
        if (pad.button[2] & PAD_DOWN)  player.body[0].y += player.speed;
        if (pad.button[2] & PAD_LEFT)  player.body[0].x -= player.speed;
        if (pad.button[2] & PAD_RIGHT) player.body[0].x += player.speed;

        // Atualizar IA inteligente da cobra inimiga
        updateAI(&enemy, player.body[0]);

        // Sair com o SELECT
        if (pad.button[2] & PAD_SELECT) {
            printf("[JOGO] A sair para a XMB...\n");
            break;
        }

        sysTimerSleep(0.05);
    }

    return 0;
}
