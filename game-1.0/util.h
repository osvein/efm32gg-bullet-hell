#define lenof(a) (sizeof(a)/sizeof(*(a)))
#define endof(a) (a + lenof(a))

typedef struct { short x, y; } Vec;

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

/* returns a vector with same angle as direction, and specified magnitude */
static inline Vec vec_setabs(Vec direction, short magnitude) {
	unsigned short factor = magnitude / vec_abs(direction);
	return (Vec){direction.x * factor, direction.y * factor};
}

/* returns the sum of two vectors */
static inline Vec vec_add(Vec a, Vec b) {
	return (Vec){a.x + b.x, a.y + b.y};
}
