#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#include "util.h"
#include "draw.h"

enum {
	LEFT = 1<<0,
	UP = 1<<1,
	RIGHT = 1<<2,
	DOWN = 1<<3,
};

typedef struct {
	Vec pos;
	Vec velocity;
	short radius;
} Bullet;

/* pool of bullet objects. the pool in divided in two, the first part are
 * allocated objects, the last part are free objects
 */
typedef struct {
	Bullet *active; /* points to the first bullet in the pool */
	Bullet *inactive; /* points to the first inactive bullet in the pool */
	Bullet *end; /* points one beyond the last bullet in the pool */
} BulletPool;

/* the complete self-contained state of the game */
typedef struct {
	int gamepad_fd;
	Vec player;
	short player_size;
	short player_speed;
	Draw draw;
	BulletPool bullets;
} Game;

char *argv0;

void fatal(void) {
	perror(argv0);
	exit(1);
}

Bullet *bulletpool_get(BulletPool* self) {
	if (self->inactive < self->end) {
		return self->inactive++;
	}
	return NULL;
}

void bulletpool_put(BulletPool *self, Bullet *b) {
	if (b < self->inactive) {
		*b = *--self->inactive;
	}
}

void player_draw(Vec self, short size, Draw *draw, unsigned long color) {
	draw_rect(draw,
    	vec_add(self, (Vec){-size, -size}),
    	vec_add(self, (Vec){size, size}),
    	color
    );
}

void game_updateplayer(Game *self, unsigned long delta) {
	unsigned char input;

	player_draw(self->player, self->player_size, &self->draw, 0x000000);
	if (read(self->gamepad_fd, &input, 1) != 1) fatal();
	Vec direction = {!(input&RIGHT)-!(input&LEFT), !(input&DOWN)-!(input&UP)};
	self->player = vec_add(self->player, vec_normalize(direction, self->player_speed*delta));
	self->player.x = MIN(MAX(self->player.x, 0+self->player_size), self->draw.max.x-self->player_size);
	self->player.y = MIN(MAX(self->player.y, 0+self->player_size), self->draw.max.y-self->player_size);
	//if (is_blank())
	player_draw(self->player, self->player_size, &self->draw, 0xFFFFFF);
}

/**
 * Generates bullets which aim for the player
*/
void generate_target_bullet(Game *game, short speed) {
	Bullet *b = bulletpool_get(&game->bullets);
	if (!b) return;
    b->pos = vec_rand(vec_zero, game->draw.max);
    b->velocity = vec_normalize(vec_add(vec_mul(b->pos, -1), game->player), speed);
}

/* returns true if bullet is on-screen */
bool bullet_draw(Bullet *self, Draw *draw, unsigned long color) {
    return draw_rect(draw,
    	vec_add(self->pos, (Vec){-10, -10}),
    	vec_add(self->pos, (Vec){10, 10}),
    	color
    );
}

/**
 * Updates all bullets loaded in game
*/
void game_updatebullets(Game *game, unsigned long delta, unsigned long color) {
	Bullet *b;
    for (b = game->bullets.active; b < game->bullets.inactive; b++) {
    	b->pos = vec_add(b->pos, vec_mul(b->velocity, delta));
		if (!bullet_draw(b, &game->draw, color)) bulletpool_put(&game->bullets, b);
    }
}

void game_tick(Game *self, unsigned long usdelta) {
//	draw_blankall(&self->draw);
	generate_target_bullet(self, 4);
	game_updatebullets(self, 0, 0x000000);
	game_updatebullets(self, usdelta, 0x0000FF);
	game_updateplayer(self, usdelta);
//	draw_commitall(&self->draw);
}

int main(int argc, char *argv[]) {
	Bullet bullet_pool[40];
	Game game = {
		.player = {100, 100},
		.player_size = 10,
		.player_speed = 10,
		.draw = {0, 0},
		.bullets = {bullet_pool, bullet_pool, endof(bullet_pool)}
	};
//	struct timespec prevtime;

	argv0 = *argv;
	game.gamepad_fd = open("/dev/gamepad", O_RDONLY);
	if (game.gamepad_fd < 0 || draw_open(&game.draw, "/dev/fb0") < 0) fatal();
	while (1) { // TODO
		// clock_gettime(CLOCK_REALTIME, ...);
		game_tick(&game, 1);
		// sleep(1);
		// clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, ...)
	}

	return 0;
}
