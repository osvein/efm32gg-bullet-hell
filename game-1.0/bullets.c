#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <game.h>

#define WIDTH  (320*64)
#define HEIGHT  (240*64)

int speed = 64;

Bullet bullets[40];

Pt randomPt(void) {
    return (Pt){
        rand() % WIDTH,
        rand() % HEIGHT
    };
}

Pt generateTargetVelocity(pt1, pt2, speed){
    
}

/**
 * Generates bullets which aim for the player
*/
void generate_target_bullets(Bullet *b) {
    b->pos = randomPt();
    Pt dist = {
        b->pos.x - player.x,
        b->pos.y - player.y
    };
    b->velocity.x = dist.x * (speed/(dist.x+dist.y));
    b->velocity.y = dist.y * (speed/(dist.x+dist.y));
}

/**
 * Generates bullets in specific patterns
*/
void generate_pattern_bullets(int index[]) {

}

/**
 * Generates bullets with semi-random velocity vectors
 * Bullets will not be generated if they would move off screen quickly
*/
void generate_random_bullets(int index) {

}

/**
 * Updates all bullets loaded in game
*/
void update_bullets(void) {
    for (Bullet *b = bullets; b < endof(bullets); b++) {
        b->pos.x += b->velocity.x;
        b->pos.y += b->velocity.y;

        if (off_screen(b)) {
            generate_target_bullets(b);
        }

        draw_bullet(b);
    }
}

/**
 * 
*/
bool off_screen(Bullet *b) {
    if (b->pos.x < 128 || b->pos.x > WIDTH - 127 ||
        b->pos.y < 128 || b->pos.y > HEIGHT - 127) {
            return true;
        }
    return false;
}

void draw_bullet(Bullet *b) {
    draw_rect((Pt){(b->pos.x)/64-2, (b->pos.y)/64-2,}, (Pt){(b->pos.x)/64+2, (b->pos.y)/64+2}, from_hex(0x0000FF));
}