#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <game.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#define WIDTH 320
#define HEIGHT 240

player = {160, 120}

uint16_t *buffer_map = mmap(NULL, WIDTH*HEIGHT*2, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

/**
 * Runs the game
 * 
 * @param {int} argc - 
 * @param {char} *argv[] - 
*/
int main(int argc, char *argv[])
{
	int fd;

	fd = open("/dev/fb0", O_RDWR);
	
	//draw_rect({50, 50}, {WIDTH-50, Height-50}, from_hex(0x00FF00));
	while (1) {
		draw_rect({0, 0}, {WIDTH, Height}, from_hex(0x000000));
		update_bullets();
		ioctl(fd, 0x4680, & (struct fb_copyarea){.dx=0, .dy=0, .width=WIDTH, .height=HEIGHT});
	}

	printf("Hello World, I'm game!\n");

	exit(EXIT_SUCCESS);
}

/**
 * Converts a point to a location in the framebuffer
 * 
 * @param {Pt} point - the point to be converted
 * @returns {int} - the appropriate location in the framebuffer
*/
int idx(Pt point) {
	return point.x + WIDTH*point.y;
}

/**
 * Draws the pixels in a rectangular area
 * 
 * @param {Pt} pt1 - the upper left limit of the area
 * @param {Pt} pt2 - the lower right limit of the area
 * @param {uint16_t} *buffer - the framebuffer of the LCD screen
 * @param {uint16_t} colour - the color to be drawn in the area 
*/
void draw_rect(Pt pt1, Pt pt2, uint16_t colour){
	Pt pixel;
	for(pixel.x = pt1.x; pixel.x <= pt2.x; pixel.x++){
		for(pixel.y = pt1.y; pixel.y <= pt2.y, pixel.y++){
			*buffer_map[idx(pixel)] = colour;
		}
	}
}

/**
 * Converts from hexcode colour to binary rgb colour
 * 
 * @param {unsigned long} hex_rgb - hexcode of the colour
 * @returns {uint_t} - binary rgb code of the colour
*/
uint16_t from_hex(unsigned long hex_rgb){
	unsigned long r = hex_rgb >> 8 & 0b1111100000000000;
	unsigned long g = hex_rgb >> 5 & 0b0000011111100000;
	unsigned long b = hex_rgb >> 3 & 0b0000000000011111;
	return r|g|b;
}

void error(void){
	perror()
	exit(1)
}

/**
 * Runs the game
 * 
 * @param {int} argc - 
 * @param {char} *argv[] - 
*/
int main(int argc, char *argv[])
{
	uint16_t *buffer_map;
	int fd;

	fd = open("/dev/fb0", O_RDWR);
	if (fd < 0){
		error();
	}
	buffer_map = mmap(NULL, WIDTH*HEIGHT*2, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (buffer_map == MAP_FAILED){
		error();
	}
	draw_rect((Pt){50, 50}, (Pt){WIDTH-50, HEIGHT-50}, buffer_map, from_hex(0x00FF00));
	ioctl(fd, 0x4680, & (struct fb_copyarea){.dx=0, .dy=0, .width=WIDTH, .height=HEIGHT});

	printf("Hello World, I'm game!\n");

	exit(EXIT_SUCCESS);
}
