#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "util.h"
#include "draw.h"

enum {
	LEFT = 1<<0,
	UP = 1<<1,
	RIGHT = 1<<2,
	DOWN = 1<<3,
};

typedef struct {
	short speed;
	short size;
	Vec pos;
} Player;

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
	struct {
		unsigned long player;
		unsigned long bullet;
	} colors;
	Player player;
	Draw draw;
	BulletPool bullets;
	int gamepad_fd;
} Game;

char *argv0;

void fatal(void) {
	perror(argv0);
	exit(1);
}

/* returns true if bullet is on-screen */
bool bullet_draw(Bullet *self, Draw *draw, unsigned long color) {
    return draw_rect(draw,
    	vec_add(self->pos, (Vec){-10, -10}),
    	vec_add(self->pos, (Vec){10, 10}),
    	color
    );
}

void player_draw(Player *self, Draw *draw, unsigned long color) {
	draw_rect(draw,
    	vec_add(self->pos, (Vec){-self->size, -self->size}),
    	vec_add(self->pos, (Vec){self->size, self->size}),
    	color
    );
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

void game_over(void) {
	printf("Game Over");
//	exit(0);
}

void game_updateplayer(Game *self, unsigned long delta) {
	Vec direction;
	unsigned char input;

	if (read(self->gamepad_fd, &input, 1) != 1) fatal();
	direction = vec_mul(
		(Vec){!(input&RIGHT)-!(input&LEFT), !(input&DOWN)-!(input&UP)},
		0x7FFF
	);

	player_draw(&self->player, &self->draw, DRAW_DIRTYONLY);
	self->player.pos = vec_add(self->player.pos,
		vec_normalize(direction, self->player.speed*delta)
	);
	/*self->player = vec_max(self->player, size);
	self->player = vec_min(self->player,
		vec_add(self->draw.max, vec_mul(size, -1)
	);*/
	self->player.pos.x = MIN(MAX(self->player.pos.x, 0+self->player.size), self->draw.max.x-self->player.size);
	self->player.pos.y = MIN(MAX(self->player.pos.y, 0+self->player.size), self->draw.max.y-self->player.size);
	player_draw(&self->player, &self->draw, self->colors.player);
	if (!draw_isblank(&self->draw,
		vec_add(self->player.pos, (Vec){-self->player.size, -self->player.size}),
		vec_add(self->player.pos, (Vec){self->player.size, self->player.size})
	)) game_over();
}

/**
 * Generates bullets which aim for the player
*/
void game_gen_target_bullet(Game *self, short speed) {
	Bullet *b = bulletpool_get(&self->bullets);
	if (!b) return;
    b->pos = vec_rand(vec_zero, self->draw.max);
    b->velocity = vec_normalize(
    	vec_add(vec_mul(b->pos, -1), self->player.pos),
    	speed
    );
}

/**
 * Generates bullets in a pattern
*/
void game_gen_pattern_bullets(Game *self, short speed) {
	if (self->bullets.end - self->bullets.inactive < 30) {
		return;
	}
	static Vec angles[] = {{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}};
	int i;
	for (i = 0; i < 8; i++) {
		Bullet *b = bulletpool_get(&self->bullets);
		b->pos = vec_rand(vec_zero, self->draw.max);
		b->velocity = vec_normalize(angles[i], speed);
	}
}

/**
 * Updates all bullets loaded in game
*/
void game_updatebullets(Game *self, unsigned long delta) {
	Bullet *b;
    for (b = self->bullets.active; b < self->bullets.inactive; b++) {
    	b->pos = vec_add(b->pos, vec_mul(b->velocity, delta));
		if (!bullet_draw(b, &self->draw, self->colors.bullet)) {
			bulletpool_put(&self->bullets, b);
		}
    }
}

void game_tick(Game *self, unsigned long usdelta) {
	draw_blankall(&self->draw);
	game_gen_pattern_bullets(self, 4);
	game_gen_target_bullet(self, 4);
	game_updatebullets(self, usdelta);
	game_updateplayer(self, usdelta);
	draw_rect(&self->draw, vec_zero, self->draw.max, DRAW_DIRTYONLY);
	draw_commit(&self->draw);
}

int main(int argc, char *argv[]) {
	struct fb_copyarea dirtylist[82];
	Bullet bullet_pool[40];
	Game game = {
		.colors = {
			.player = 0xFFFFFFul,
			.bullet = 0xFF0000ul
		},
		.player = {.speed = 1, .size = 10},
		.draw = {0, 0, dirtylist, lenof(dirtylist)},
		.bullets = {bullet_pool, bullet_pool, endof(bullet_pool)},
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
