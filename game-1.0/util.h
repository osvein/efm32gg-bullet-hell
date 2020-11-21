#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN(a, b) ((a)<(b)?(a):(b))
#define MAX(a, b) ((a)>(b)?(a):(b))
#define lenof(a) (sizeof(a)/sizeof(*(a)))
#define endof(a) (a + lenof(a))

typedef struct { short x, y; } Vec;
typedef struct { Vec from, to; } Rect; /* both inclusive */

static const Vec vec_zero = {0, 0};
static const Rect rect_empty = {vec_zero, {-1, -1}};

/* returns integer square root of x, capped at 2^bits
 * fast, gcc tends to unroll with constant bits at -O3
 */
static inline unsigned short isqrt(unsigned long x, unsigned bits) {
	unsigned y = 0;

	while (bits-- > 0) {
		unsigned a = (y + (1 << bits)) << bits;

		if (x >= a) {
			x -= a;
			y |= 2 << bits;
		}
	}
	return y >> 1;
}

/* returns the absolute value (length) of vector v */
static inline unsigned short vec_abs(Vec v) {
	return isqrt((unsigned long)v.x * v.x + (unsigned long)v.y * v.y, CHAR_BIT*sizeof(v.x));
}

/* returns the sum of two vectors, overflow-unsafe */
static inline Vec vec_add(Vec a, Vec b) {
	return (Vec){a.x + b.x, a.y + b.y};
}

/* multiplies both coordinates of v by factor, overflow-usafe */
static inline Vec vec_mul(Vec v, short factor) {
	return (Vec){v.x * factor, v.y * factor};
}

/* returns a vector with same angle as v, and specified magnitude */
static inline Vec vec_normalize(Vec v, short magnitude) {
	short div = vec_abs(v);

	/* can't use vec_mul due to overflow */
	return (Vec){((long)v.x * magnitude) / div, ((long)v.y * magnitude) / div};
}

/* returns a random vector inside a rectangle using rand() */
static inline Vec vec_rand(Vec min, Vec max) {
	return (Vec){
		rand() % ((long)max.x + 1) + min.x,
		rand() % ((long)max.y + 1) + min.y
	};
}

static inline Vec vec_min(Vec a, Vec b) {
	return (Vec){a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y};
}

static inline Vec vec_max(Vec a, Vec b) {
	return (Vec){a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y};
}

/*
static inline Rect rect(Vec a, Vec b) {
	return (Rect){vec_min(a, b), vec_max(a, b)};
}

static inline unsigned long rect_width(Rect r) {
	return (unsigned long)r.b.x - r.a.x + 1;
}

static inline unsigned long rect_height(Rect r) {
	return (unsigned long)r.b.y - r.a.y + 1;
}
*/
