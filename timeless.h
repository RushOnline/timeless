/* data.c stuff */

#define WIDTH 400

/* stuff from tiles.m */
extern unsigned char gradient_sphere[];		// tile00-tile03
extern unsigned char daisy[];			// tile04-tile07
extern unsigned char lightning[]; 		// tile08-tile0b
extern unsigned char star_thingy[];		// tile0c-tile0f
extern unsigned char small_gradient_sphere[];	// tile10
extern unsigned char eyeball[];			// tile11-tile14
extern unsigned char face[];			// tile15-tile18
extern unsigned char sundial[];			// tile19-tile1c

/* stuff from pals.m */
extern unsigned char pal00[128*3]; // white
extern unsigned char pal01[128*3]; // blue
extern unsigned char pal02[128*3]; // green
extern unsigned char pal03[128*3]; // red
extern unsigned char pal04[128*3]; // light blue
extern unsigned char pal05[128*3]; // light green
extern unsigned char pal06[128*3]; // magenta
extern unsigned char pal07[128*3]; // blue/purple
extern unsigned char pal08[128*3]; // green/light cyan
extern unsigned char pal09[128*3]; // red/yellow
extern unsigned char pal0a[];	// misc
extern unsigned char pal0b[128*3]; // violet
extern unsigned char pal0c[128*3]; // blue green
extern unsigned char pal0d[128*3]; // orange
extern unsigned char pal0e[128*3]; // blue/white
extern unsigned char pal0f[128*3]; // green/white
extern unsigned char pal10[128*3]; // red/white
extern unsigned char _bpal[256*3];

/* random data */
extern unsigned char offscreen[256*256];
extern unsigned char gnbgtbl[64000];

/* from sprites.c */
extern unsigned char cspr00[];
extern unsigned char cspr01[];
extern unsigned char cspr02[];
extern unsigned char cspr03[];
extern unsigned char cspr04[];
extern unsigned char cspr05[];
extern unsigned char cspr06[];
extern unsigned char cspr07[];
extern unsigned char cspr08[];
extern unsigned char cspr09[];
extern unsigned char cspr0a[];
extern unsigned char cspr0b[];
extern unsigned char cspr0c[];
extern unsigned char cspr0d[];
extern unsigned char cspr0e[];
extern unsigned char cspr0f[];
extern unsigned char cspr10[];
extern unsigned char cspr11[];
extern unsigned char cspr12[];
extern unsigned char cspr13[];
extern unsigned char cspr14[];
extern unsigned char cspr15[];
extern unsigned char cspr16[];
extern unsigned char cspr17[];
extern unsigned char cspr18[];
extern unsigned char cspr19[];
extern unsigned char cspr1a[];
extern unsigned char cspr1b[];
extern unsigned char cspr1c[];
extern unsigned char cspr1d[];
extern unsigned char cspr1e[];
extern unsigned char cspr1f[];
extern unsigned char cspr20[];
extern unsigned char cspr21[];
extern unsigned char cspr22[];

extern signed char sincos0s[];
extern unsigned char sincos0[];

/* sound.c */

extern void *sound_init(void);
extern void sound_pulse(void *data);

/* song.c */

extern unsigned char song[];
extern int song_size;
