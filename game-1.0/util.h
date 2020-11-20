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
static inline unsigned isqrt(unsigned x, unsigned bits) {
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
	return isqrt(v.x * v.x + v.y * v.y, 16);
}

/* returns the sum of two vectors */
static inline Vec vec_add(Vec a, Vec b) {
	return (Vec){a.x + b.x, a.y + b.y};
}

/* multiplies both coordinates of v by a factor */
static inline Vec vec_mul(Vec v, short factor) {
	return (Vec){v.x * factor, v.y * factor};
}

/* returns a vector with same angle as direction, and specified magnitude */
static inline Vec vec_normalize(Vec direction, short magnitude) {
	return vec_mul(direction, magnitude / vec_abs(direction));
}

static inline Vec vec_rand(Vec max) {
	return (Vec){rand() % max.x, rand() % max.y};
}
