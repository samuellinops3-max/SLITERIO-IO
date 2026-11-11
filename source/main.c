#include <psl1ght/lv2.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/sys_time.h>
#include <pad.h>
#include <gcm.h>

#define SCREEN_W 1920
#define SCREEN_H 1080
#define MAP_SIZE 4000.0f
#define SEGMENTS_MAX 100
#define FOOD_MAX 200
#define BOTS_MAX 3

typedef struct { float x, y; } Point;
typedef struct {
    Point pos[SEGMENTS_MAX];
    int length;
    float angle;
    float speed;
    float radius;
    int alive;
    int color;
    float boostEnergy;
} Snake;

typedef struct {
    float x, y;
    float radius;
    int color;
    int alive;
} Food;

Snake player;
Snake bots[BOTS_MAX];
Food food[FOOD_MAX];
PadData pad;
float cameraX, cameraY;
int score = 0;

#define RGBA(r,g,b,a) (((a)<<24)|((r)<<16)|((g)<<8)|(b))
#define COLOR_CYAN  RGBA(0,255,255,255)
#define COLOR_BLUE  RGBA(0,128,255,255)
#define COLOR_RED   RGBA(255,64,64,255)
#define COLOR_GREEN RGBA(64,255,64,255)
#define COLOR_PURP  RGBA(192,64,255,255)
#define COLOR_BG    RGBA(16,16,24,255)

float dist(float x1, float y1, float x2, float y2) {
    return sqrtf((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}

float randRange(float min, float max) {
    return min + (float)rand()/RAND_MAX*(max-min);
}

void initSnake(Snake *s, float x, float y, int color) {
    s->length = 12;
    s->pos[0].x = x; s->pos[0].y = y;
    for(int i=1; i<s->length; i++) {
        s->pos[i].x = x - i*5.0f;
        s->pos[i].y = y;
    }
    s->angle = 0;
    s->speed = 3.0f;
    s->radius = 7.0f;
    s->alive = 1;
    s->color = color;
    s->boostEnergy = 100.0f;
}

void spawnFood(int i) {
    food[i].x = randRange(-MAP_SIZE/2, MAP_SIZE/2);
    food[i].y = randRange(-MAP_SIZE/2, MAP_SIZE/2);
    food[i].radius = randRange(2.5f, 5.0f);
    int c = rand()%5;
    food[i].color = c==0?RGBA(255,128,128,255):
                     c==1?RGBA(128,255,128,255):
                     c==2?RGBA(128,128,255,255):
                     c==3?RGBA(255,255,128,255):
                          RGBA(255,128,255,255);
    food[i].alive = 1;
}

void initGame() {
    srand(sys_time_get_system_time());
    initSnake(&player, 0, 0, COLOR_CYAN);
    int botColors[] = {COLOR_BLUE, COLOR_RED, COLOR_PURP};
    for(int i=0; i<BOTS_MAX; i++) {
        float rx = randRange(-MAP_SIZE/3, MAP_SIZE/3);
        float ry = randRange(-MAP_SIZE/3, MAP_SIZE/3);
        initSnake(&bots[i], rx, ry, botColors[i]);
    }
    for(int i=0; i<FOOD_MAX; i++) spawnFood(i);
    cameraX = player.pos[0].x;
    cameraY = player.pos[0].y;
    score = 0;
}

void updateSnake(Snake *s, float targetAngle, float speedMult) {
    if(!s->alive) return;
    float diff = targetAngle - s->angle;
    while(diff>M_PI) diff-=6.283f;
    while(diff<-M_PI) diff+=6.283f;
    s->angle += diff * 0.15f;
    float spd = s->speed * speedMult;
    s->pos[0].x += cosf(s->angle) * spd;
    s->pos[0].y += sinf(s->angle) * spd;
    float half = MAP_SIZE/2;
    if(s->pos[0].x < -half) s->pos[0].x = -half;
    if(s->pos[0].x > half) s->pos[0].x = half;
    if(s->pos[0].y < -half) s->pos[0].y = -half;
    if(s->pos[0].y > half) s->pos[0].y = half;
    for(int i=1; i<s->length; i++) {
        float dx = s->pos[i-1].x - s->pos[i].x;
        float dy = s->pos[i-1].y - s->pos[i].y;
        float d = sqrtf(dx*dx+dy*dy);
        float ideal = s->radius * 1.2f;
        if(d > ideal) {
            s->pos[i].x += dx * 0.25f;
            s->pos[i].y += dy * 0.25f;
        }
    }
}

void checkFood(Snake *snake) {
    for(int i=0; i<FOOD_MAX; i++) {
        if(!food[i].alive) continue;
        float d = dist(snake->pos[0].x, snake->pos[0].y, food[i].x, food[i].y);
        if(d < snake->radius + food[i].radius) {
            food[i].alive = 0;
            if(snake->length < SEGMENTS_MAX) snake->length++;
            snake->radius += 0.04f;
            if(snake == &player) score += 10;
            spawnFood(i);
        }
    }
}

int checkCollision(Snake *a, Snake *b) {
    for(int i=1; i<b->length; i++) {
        if(dist(a->pos[0].x,a->pos[0].y, b->pos[i].x,b->pos[i].y) < a->radius*0.7f)
            return 1;
    }
    return 0;
}

void updateBot(Snake *bot) {
    if(!bot->alive) return;
    float tx = bot->pos[0].x + 100, ty = bot->pos[0].y, nearD=999999;
    for(int i=0; i<FOOD_MAX; i++) {
        if(!food[i].alive) continue;
        float d = dist(bot->pos[0].x,bot->pos[0].y, food[i].x,food[i].y);
        if(d < nearD) { nearD=d; tx=food[i].x; ty=food[i].y; }
    }
    float ang = atan2f(ty - bot->pos[0].y, tx - bot->pos[0].x);
    updateSnake(bot, ang, 1.0f);
    checkFood(bot);
}

void handleInput(float *ang, float *spd) {
    padRead(0, &pad);
    float lx = pad.analog[0]-128;
    float ly = pad.analog[1]-128;
    *spd = 1.0f;
    if((pad.button & PAD_BUTTON_CROSS) && player.boostEnergy>0) {
        *spd = 2.0f;
        player.boostEnergy -= 0.25f;
        if(rand()%25==0 && player.length>8) player.length--;
    } else if(player.boostEnergy<100) player.boostEnergy += 0.1f;
    if(fabsf(lx)>8 || fabsf(ly)>8) *ang = atan2f(ly, lx);
}

int main() {
    gcmInit(); padInit(); atexit(gcmExit);
    initGame();
    float ang=0, spd=1.0f;
    while(1) {
        handleInput(&ang, &spd);
        if(player.alive) {
            updateSnake(&player, ang, spd);
            checkFood(&player);
            for(int i=0; i<BOTS_MAX; i++) {
                if(checkCollision(&player, &bots[i])) player.alive=0;
                if(checkCollision(&bots[i], &player)) {
                    bots[i].alive=0;
                    for(int s=0;s<bots[i].length;s++) {
                        int fi=rand()%FOOD_MAX;
                        food[fi].x=bots[i].pos[s].x;
                        food[fi].y=bots[i].pos[s].y;
                        food[fi].alive=1;
                    }
                }
            }
        }
        for(int i=0; i<BOTS_MAX; i++) {
            if(!bots[i].alive && rand()%500==0) {
                float rx=randRange(-MAP_SIZE/3,MAP_SIZE/3);
                float ry=randRange(-MAP_SIZE/3,MAP_SIZE/3);
                initSnake(&bots[i],rx,ry,bots[i].color);
            }
            updateBot(&bots[i]);
        }
        cameraX += (player.pos[0].x - cameraX) * 0.08f;
        cameraY += (player.pos[0].y - cameraY) * 0.08f;
        gcmSetClearColor(COLOR_BG);
        gcmSwapBuffers(1);
        sys_usleep(16000);
    }
    return 0;
}
