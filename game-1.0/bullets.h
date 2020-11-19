typedef struct {
	Vec pos;
	Vec velocity;
} Bullet;

typedef struct {
	Bullet *pool;
	unsigned capacity;
	unsigned nactive;
}

void bullets_update(Bullets *self);
