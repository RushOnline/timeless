#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "scaler.h"

typedef struct scaler {
	void (*callback)(void *data, unsigned char *row, int len);
	void *callback_data;
	int sw,sh;
	int dw,dh;
	int bpp;
	unsigned char *accum;
	unsigned char *output;
	int yf,yr;
	int bytewidth;
	int obytewidth;
} scaler;


static inline void accumrow(unsigned char *d, int w, unsigned char *s,
		int top, int bottom)
{
int f;
	f=0x100*top/bottom;
	while(w--)
		d[w] += s[w]*f>>8;
}

static inline void accumpixel(unsigned char *d, int bpp, unsigned char *s,
		int top, int bottom)
{
int f=0x100*top/bottom;
	while(bpp--)
		d[bpp] += s[bpp]*f>>8;
}

static inline void xscale(unsigned char *d, int w2, unsigned char *s, int w1, int bpp)
{
int pixelcount;
int xf,xr;

	pixelcount=w2;

	xf=w2;
	xr=w1;

	memset(d, 0, w2*bpp);
	while(pixelcount)
	{
		if(xr>xf)
		{
			accumpixel(d, bpp, s, xf, w1);
			xr-=xf;
			xf=0;
		} else
		{
			accumpixel(d, bpp, s, xr, w1);
			xf-=xr;
			xr=0;

		}
		if(xr==0) // we can kick out a pixel
		{
			d+=bpp;
			xr=w1;
			--pixelcount;
		}
		if(xf==0) // we can step the source pointer
		{
			s+=bpp;
			xf=w2;
		}
	}
}

void *scaler_alloc(int dw, int dh, int sw, int sh, int bpp,
		void (*callback)(void *data, unsigned char *row, int len),
		void *data)
{
scaler *s;
	s=malloc(sizeof(scaler));
	if(s)
	{
		s->bytewidth = sw * bpp;
		s->obytewidth = dw * bpp;
		s->accum = malloc(s->bytewidth);
		s->output = malloc(s->obytewidth);
		s->sw = sw;
		s->sh = sh;
		s->dw = dw;
		s->dh = dh;
		s->bpp = bpp;
		s->callback = callback;
		s->callback_data = data;
		s->yf=dh;
		s->yr=sh;
		if(s->accum && s->output)
			memset(s->accum, 0, s->bytewidth);
		else
		{
			if(s->accum)
				free(s->accum);
			if(s->output)
				free(s->output);
			free(s);
			s=0;
		}
	}
 	return s;
}

void scaler_free(void *p)
{
scaler *s=p;
	if(s)
	{
		if(s->accum)
			free(s->accum);
		if(s->output)
			free(s->output);
		free(s);
	}
}

void scaler_feed(void *p, unsigned char *row)
{
scaler *s=p;

	while(s)
	{
		if(s->yr > s->yf)
		{
			accumrow(s->accum, s->bytewidth, row, s->yf, s->sh);
			s->yr -= s->yf;
			s->yf = 0;
		} else
		{
			accumrow(s->accum, s->bytewidth, row, s->yr, s->sh);
			s->yf -= s->yr;
			s->yr = 0;
		}
		if(s->yr == 0) // we can kick out a line
		{
			xscale(s->output, s->dw, s->accum, s->sw, s->bpp);
			memset(s->accum, 0, s->bytewidth);
			s->yr = s->sh;
			s->callback(s->callback_data, s->output, s->obytewidth);
		}
		if(s->yf == 0) // we're done with the input line.
		{
			s->yf = s->dh;
			break;
		}
	}
}
