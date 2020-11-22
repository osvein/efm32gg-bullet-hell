#include <linux/fb.h>
#include <stdbool.h>
#include <stdint.h>

enum {
	DRAW_DIRTYONLY = 1<<24
};

typedef struct {
	unsigned char subwidth, subheight; /* log2(subpixels per pixel) */
	struct fb_copyarea *dirtylist;
	size_t dirtylist_cap;
	int fd;
	Vec max; /* display size in subpixels */
	size_t stride; /* bytes per line */
	size_t bufsize;
	uint16_t *buf;
	size_t dirtylist_len;
} Draw;

/* returns true iff all pixels in the area are not bullet_color */
bool draw_isblank(Draw *self, Vec pt, Vec size, unsigned long bullet_color);

/* writes black to all pixels */
void draw_blankall(Draw *self);

/* commits the dirtylist */
void draw_commit(Draw *self);

void draw_unmap(Draw *self);
void draw_close(Draw *self);

/* retrieves info and maps framebuffer, returns <0 on error */
int draw_init(Draw *self);

/* opens framebuffer at path for drawing, returns <0 on error */
int draw_open(Draw *self, const char *path);

/* Draws the pixels in a rectangular area */
bool draw_rect(Draw *self, Vec pt1, Vec pt2, unsigned long colour);

/* Converts from hexcode colour to binary rgb colour */
 
static inline uint16_t draw_convcolor(unsigned long hex_rgb){
	unsigned long r = hex_rgb >> 8 & 0x1F << 11;
	unsigned long g = hex_rgb >> 5 & 0x3f << 5;
	unsigned long b = hex_rgb >> 3 & 0x1F;
	return r|g|b;
}
