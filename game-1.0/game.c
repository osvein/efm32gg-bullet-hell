#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <game.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#include "util.h"
#include "draw.h"
#include "bullets.h"

typedef struct {
	int gamepad_fd;
	Vec player;
	Draw draw;
	Bullets bullets;
} Game;

void game_moveplayer(Game *self) {
	// TODO
}

void game_drawplayer(Game *self) {
	// TODO
}

void game_tick(Game *self, unsigned long usdelta) {
	bullets_update(&self->bullets);
	game_moveplayer(self);
	game_drawplayer(self);
	draw_commitall(&self->draw);
	draw_blankall(&self->draw);
}

/**
 * Runs the game
 * 
 * @param {int} argc - 
 * @param {char} *argv[] - 
*/
int main(int argc, char *argv[]) {
	Bullet bullet_pool[40];
	Game game = {
		.draw = {6, 6}
		.bullets = {bullet_pool, lenof(bullet_pool)}
	};
	struct timespec prevtime;

	game.gamepad_fd = open("/dev/gamepad", O_RDONLY);
	if (game.gamepad_fd < 0 || draw_open(&game.draw, "/dev/fb0") < 0) {
		perror(*argv);
		exit(1);
	}
	while (1) { // TODO
		// clock_gettime(CLOCK_REALTIME, ...), regn ut prevtime diff
		game_tick(&game, delta);
		// clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, ...)
	}

	return 0;
}
