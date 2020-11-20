#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN(a, b) ((a)<(b)?(a):(b))
#define MAX(a, b) ((a)>(b)?(a):(b))
#define lenof(a) (sizeof(a)/sizeof(*(a)))
#define endof(a) (a + lenof(a))

typedef struct { short x, y; } Vec;

static const Vec vec_zero = {0, 0};

/* returns integer square root of x, capped at 2^bits
 * fast, gcc tends to unroll with constant bits at -O3
 */
static inline unsigned long isqrt(unsigned long x, unsigned bits) {
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
static inline short vec_abs(Vec v) {
	return isqrt(v.x * v.x + v.y * v.y, 15);
}

/* returns the sum of two vectors */
static inline Vec vec_add(Vec a, Vec b) {
	return (Vec){a.x + b.x, a.y + b.y};
}

/* multiplies both coordinates of v by a factor */
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
