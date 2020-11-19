#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

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

/**
 * Converts a point to a location in the framebuffer
 *
 * @param {Vec} point - the point to be converted
 * @returns {int} - the appropriate location in the framebuffer
*/
static unsigned draw_getidx(Draw *self, Vec px) {
	return px.x + (self->stride/2) * px.y;
}

bool draw_isblank(Draw *self, Vec pt1, Vec pt2) {
	// TODO
}

void draw_blankall(Draw *self) {
	memset(self->buf, 0, self->bufsize);
}

void draw_commit(Draw *self, Vec pt1, Vec pt2) {
	pt1 = draw_downscale(self, pt1);
	pt2 = draw_downscale(self, pt2);
	ioctl(self->fd, 0x4680, &(struct fb_copyarea){
		.dx = pt1.x, .dy = pt1.y, .width = pt2.x-pt1.x, .height = pt2.y-pt1.y
	});
}

void draw_commitall(Draw *self) {
	draw_commit(self, vec_zero, self->size);
}

void draw_unmap(Draw *self) {
	if (self->buf) munmap(self->buf, self->bufsize);
}

void draw_close(Draw *self) {
	draw_unmap(self);
	if (self->fd >= 0) close(self->fd);
}

int draw_map(Draw *self) {
	{
		struct fb_fix_screeninfo info;
		if (ioctl(self->fd, FBIOGET_FSCREENINFO, &info) < 0) return -1;
		self->stride = info.line_length;
		self->bufsize = info.smem_len;
	}
	{
		struct fb_var_screeninfo info;
		if (ioctl(self->fd, FBIOGET_VSCREENINFO, &info) < 0) return -1;
		self->size = draw_upscale(self, (Vec){info.xres, info.yres});
	}
	self->buf = mmap(NULL, self->bufsize, PROT_READ|PROT_WRITE, MAP_SHARED,
		self->fd, 0
	);
	if (self->buf == MAP_FAILED) {
		self->buf = NULL;
		return -1;
	}
	return 0;
}

int draw_open(Draw *self, const char *path) {
	self->fd = open(path, O_RDWR);
	if (self->fd < 0) return -1;
	if (draw_map(self) < 0) {
		draw_close(self);
		return -1;
	}
	return 0;
}

bool draw_rect(Draw *self, Vec pt1, Vec pt2, uint16_t colour){
	Vec pixel;

	pt1 = draw_downscale(self, pt1);
	pt2 = draw_downscale(self, pt2);
	for(pixel.x = pt1.x; pixel.x <= pt2.x; pixel.x++){
		for(pixel.y = pt1.y; pixel.y <= pt2.y; pixel.y++){
			self->buf[draw_getidx(self, pixel)] = colour;
		}
	}
}
