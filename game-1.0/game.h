#define lenof(a) (sizeof(a)/sizeof(*(a)))
#define endof(a) (a + lenof(a))

typedef struct {
	int x;
	int y;
} Pt;

typedef struct {
	Pt pos;
	Pt velocity;
} Bullet;

extern Pt player;
void draw_rect(Pt pt1, Pt pt2, uint16_t colour);
uint16_t from_hex(unsigned long hex_rgb);

void update_bullets();