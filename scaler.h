void *scaler_alloc(int dw, int dh, int sw, int sh, int bpp,
	void (*callback)(void *data, unsigned char *row, int len),
	void *data);

void scaler_free(void *p);

void scaler_feed(void *p, unsigned char *row);

