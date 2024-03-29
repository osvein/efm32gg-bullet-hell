#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

#include "util.h"
#include "draw.h"

/* by convention, Vecs named pt count in subpixels, and px count in pixels */

/* converts subpixel vector to pixel */
static Vec draw_downscale(Draw *self, Vec pt) {
	return (Vec){pt.x >> self->subwidth, pt.y >> self->subheight};
}

/* converts pixel vector to subpixel */
static Vec draw_upscale(Draw *self, Vec px) {
	return (Vec){px.x << self->subwidth, px.y << self->subheight};
}

/* Converts a point to a location in the framebuffer */
static unsigned draw_getidx(Draw *self, Vec px) {
	return px.x + (self->stride/2) * px.y;
}

/* returns true iff all pixels in the area are not bullet_color */
bool draw_isblank(Draw *self, Vec pt1, Vec pt2, unsigned long bullet_color) {
	Vec pixel;
	unsigned long conv_color = draw_convcolor(bullet_color);
	pt1 = draw_downscale(self, pt1);
	pt2 = draw_downscale(self, pt2);
	for(pixel.x = pt1.x; pixel.x <= pt2.x; pixel.x++){
		for(pixel.y = pt1.y; pixel.y <= pt2.y; pixel.y++){
			if(self->buf[draw_getidx(self, pixel)] == conv_color) return false;
		}
	}
	return true;
}

/* writes black to all pixels */
void draw_blankall(Draw *self) {
	memset(self->buf, 0, self->bufsize);
}

/* commits the dirtylist */
void draw_commit(Draw *self) {
	while (self->dirtylist_len > 0) {
		ioctl(self->fd, 0x4680, &self->dirtylist[--self->dirtylist_len]);
	}
}

void draw_unmap(Draw *self) {
	if (self->buf) munmap(self->buf, self->bufsize);
}

void draw_close(Draw *self) {
	draw_unmap(self);
	if (self->fd >= 0) close(self->fd);
}

/* retrieves info and maps framebuffer, returns <0 on error */
int draw_init(Draw *self) {
	struct fb_copyarea blank;
	{
		struct fb_fix_screeninfo info;
		if (ioctl(self->fd, FBIOGET_FSCREENINFO, &info) < 0) return -1;
		self->stride = info.line_length;
		self->bufsize = info.smem_len;
	}
	{
		struct fb_var_screeninfo info;
		if (ioctl(self->fd, FBIOGET_VSCREENINFO, &info) < 0) return -1;
		blank = (struct fb_copyarea){.width = info.xres, .height = info.yres};
		self->max = vec_add(
			draw_upscale(self, (Vec){info.xres, info.yres-1}),
			(Vec){-1, -1}
		);
	}
	self->buf = mmap(NULL, self->bufsize, PROT_READ|PROT_WRITE, MAP_SHARED,
		self->fd, 0
	);
	if (self->buf == MAP_FAILED) {
		self->buf = NULL;
		return -1;
	}
	draw_blankall(self);
	ioctl(self->fd, 0x4680, &blank);
	return 0;
}

/* opens framebuffer at path for drawing, returns <0 on error */
int draw_open(Draw *self, const char *path) {
	self->fd = open(path, O_RDWR);
	if (self->fd < 0) return -1;
	if (draw_init(self) < 0) {
		draw_close(self);
		return -1;
	}
	return 0;
}

/* Draws the pixels in a rectangular area and adds them to the dirtylist*/
bool draw_rect(Draw *self, Vec pt1, Vec pt2, unsigned long color){
	Vec pixel;
	uint16_t c = draw_convcolor(color);

	pt1.x = MAX(pt1.x, 0);
	pt1.y = MAX(pt1.y, 0);
	pt2.x = MIN(pt2.x, (self->max.x));
	pt2.y = MIN(pt2.y, (self->max.y));
	if (pt1.x > pt2.x || pt1.y > pt2.y) return false;
	pt1 = draw_downscale(self, pt1);
	pt2 = draw_downscale(self, pt2);

	if (self->dirtylist_len >= self->dirtylist_cap) return false;
	self->dirtylist[self->dirtylist_len++] = (struct fb_copyarea){
		.dx = pt1.x, .width = pt2.x-pt1.x+1,
		.dy = pt1.y, .height = pt2.y-pt1.y+2 
	};
	if (color & DRAW_DIRTYONLY) return true;

	for (pixel.x = pt1.x; pixel.x <= pt2.x; pixel.x++) {
		for (pixel.y = pt1.y; pixel.y <= pt2.y; pixel.y++) {
			self->buf[draw_getidx(self, pixel)] = c;
		}
	}
	return true;
}
