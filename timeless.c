#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "SDL.h"

#include "timeless.h"
#include "scaler.h"

#define XSIZE 800
#define YSIZE 500

int xsize=XSIZE;
int ysize=YSIZE;

SDL_Surface *thescreen;

#define ON_W 400
#define ON_H 280

int fast=0;

unsigned char offscreen[256*256];
unsigned char onscreen_buffer[ON_W * ON_H];
unsigned char *onscreen = onscreen_buffer + (ON_W-320)/2 + ON_W*((ON_H-200)/2);

#define OPTION_UNWARP_BG    1
#define OPTION_FAST         2
#define OPTION_PAUSED       4

void clear(void)
{
	memset(thescreen->pixels,0,XSIZE*YSIZE*2);
}

void scrlock(void)
{
	if ( SDL_LockSurface(thescreen) < 0 )
	{
		fprintf(stderr, "Couldn't lock display surface: %s\n",
							SDL_GetError());
	}
}
void scrunlock(void)
{
	SDL_UnlockSurface(thescreen);
}

void update(void)
{
	SDL_UpdateRect(thescreen, 0, 0, 0, 0);
}


unsigned long maprgb(int r,int g,int b)
{
	return SDL_MapRGB(thescreen->format,r,g,b);
}

/*****************************************************
 *****************************************************
 Graphics helper functions
 ****************************************************
 ****************************************************/

unsigned char *d3tile;
int d3counter;
unsigned char *d8tile;
int delaycntr;
unsigned char *pal0aptr = pal0a;

void r_m00(void);
void r_m01(void);
void r_m02(void);
void r_m03(void);
void r_m04(void);
void r_m05(void);
void r_m06(void);
void r_m07(void);
void r_m08(void);
void r_m09(void);
void r_m0a(void);
void r_m0b(void);
void r_m0c(void);
void r_m0d(void);
void r_m0e(void);
void r_m0f(void);
void r_m10(void);
void r_m11(void);
void r_m12(void);
void r_m13(void);
void r_m14(void);
void r_m15(void);
void r_m16(void);

void r_p00(void);
void r_p10(void);
void r_p20(void);
void r_p30(void);
void r_p40(void);
void r_p50(void);
void r_p60(void);
void r_p70(void);
void r_p80(void);
void r_p90(void);
void waitpals(void);

void r_delay(void);
void r_fast(void);
void r_slow(void);
void setnext(void);
void setstart(void);
void restart(void);

void l1start(void);
void l2start(void);

int l1on=0;
int l2on=0;

void (*routtbl[])(void)={
//r_fast,
	r_m00, r_p00, r_m01, r_p10, r_m02, waitpals,
	setstart,
	r_m03, r_p20, r_m04, waitpals, r_m05,
	r_delay, r_p30,
	r_m06, r_p30,
	r_m07, r_p30,
	r_m08, waitpals,
	r_p40, r_delay,
	r_m09, r_m0a, r_p30, r_delay,
	r_p50, r_m0b, r_delay, r_m0c, waitpals,
	r_p60, r_m0d,
	r_p70, r_delay,
	r_m0e, r_p30, r_delay,
	r_m0f, r_p80,
	r_delay,
	r_m10, r_p30, r_m11, r_p30,
	r_p90, r_delay, r_m12,
	r_p70,
	r_m13, r_p20, r_m14, waitpals,
	r_p60, r_m15, r_p30, r_delay,
	r_m16, r_p30, r_delay,
	restart
};

int routtblindex=0;
int routtblstart=0;
void (*l0rout)(void) = setnext;
void (*l1rout)(void) = l1start;
void (*l2rout)(void) = l2start;

void setstart(void)
{
	routtblstart=routtblindex;
	setnext();
}

void restart(void)
{
	routtblindex=routtblstart;
	pal0aptr += 64*3;
	if(pal0aptr == pal0a + 64*3*6)
		pal0aptr = pal0a;
	setnext();
}

void setnext(void)
{
//printf("next!\n");
	l0rout = routtbl[++routtblindex];

}

void r_delay(void)
{
	if(!--delaycntr)
		setnext();
}


/*****************************************************
 *****************************************************
 Graphics routines, l0 stuff
 ****************************************************
 ****************************************************/

// sprite structure:
// 1 byte of # of pixels in this strip
// x coordinate
// y coordinate
// 1 byte for each pixel in strip
// 0 byte = endmark
void drawsprite(unsigned char *dest, unsigned char *sp)
{
int c;
	while((c=*sp))
	{
		memcpy(dest + sp[1] + sp[2]*400, sp+3, c);
		sp+=c+3;
	}
}

// from l0p0.m
void r_p00(void)
{
static unsigned short loc=0x357c;
static unsigned short locd=0x137e;
static unsigned char color=0;
static unsigned char counter=0;

int i;
	locd += 0x1395;
	for(i=0;i<32;++i)
	{
		offscreen[loc&0xffff] = color;
		color = (color+13) & 0x3f;
		if(locd&0x8000)
		{
			++loc;
			locd <<=1;
			++locd;
		} else
			locd<<=1;
		loc+=locd;
	}
	counter -= 2;
	if(!counter) setnext();

}

// from l0p1.m
void r_p10(void)
{
static unsigned char counter=0;
int i;
int ax;
int bx;

	ax=0x40;
	bx=counter;
	for(i=0;i<16;++i)
	{
		bx&=0xffff;
		offscreen[(bx  )&0xffff] = ax;
		offscreen[(bx+1)&0xffff] = ax;
		offscreen[(bx+256  )&0xffff] = ax;
		offscreen[(bx+257)&0xffff] = ax;
		ax += 0x4;
		bx += 97*256+97;
	}
	ax=0x42;
	bx = 0x3300 + ((-counter)&255);
	for(i=0;i<16;++i)
	{
		offscreen[(bx  )&0xffff] = ax;
		offscreen[(bx+1)&0xffff] = ax;
		offscreen[(bx+256  )&0xffff] = ax;
		offscreen[(bx+257)&0xffff] = ax;
		ax+=0x4;
		bx += 17*256+223;
	}
	ax=0x41;
	bx = (counter<<8) | 0x57;
	for(i=0;i<16;++i)
	{
		offscreen[(bx  )&0xffff] = ax;
		offscreen[(bx+1)&0xffff] = ax;
		offscreen[(bx+256  )&0xffff] = ax;
		offscreen[(bx+257)&0xffff] = ax;
		ax+=0x4;
		bx+=111*256+17;
	}
	ax=0x43;
	bx=(-counter<<8) | 0xa9;
	for(i=0;i<16;++i)
	{
		offscreen[(bx  )&0xffff] = ax;
		offscreen[(bx+1)&0xffff] = ax;
		offscreen[(bx+256  )&0xffff] = ax;
		offscreen[(bx+257)&0xffff] = ax;
		ax+=0x4;
		bx+=203*256+117;
	}

	if(!--counter)
		setnext();
}

// from l0p2.m crossing lines, fill whole screen
void r_p20(void)
{
static unsigned char counter=0;
unsigned char al;
int i;
unsigned char *p;

// 80, 81...bf..bf..be...80
	al = counter<64 ? counter+0x80 : 0xff-counter;
	p=offscreen+counter*2;
	for(i=0;i<0x100;++i)
		p[i*0x100] = al;
	p=offscreen+counter*0x200;
	memset(p, al, 256);
	p=offscreen+(counter^0x7f)*0x200 + 0x100;
	memset(p, al, 256);
	p=offscreen+(counter^0x7f)*2 + 1;
	for(i=0;i<0x100;++i)
		p[i*0x100] = al;
	counter = (counter+1) & 0x7f;
	if(!counter) setnext();
}

// from l0p4.m
void r_p40(void)
{
static unsigned short loc=0x7e7e;
static short locd=-2;
static unsigned char len=1;
static unsigned char lend=1;
int color;
int i;
unsigned short si,di;

	color = (0x41 - len) & 0xff;
	if(color<0x40)
		++color;
	color+=0x3f;
	di=si=loc;
	for(i=0;i<len;++i)
	{
		offscreen[0xffff & (di+0)] = color;
		offscreen[0xffff & (di+1)] = color;
		offscreen[0xffff & (si+0)] = color;
		offscreen[0xffff & (si+1)] = color;
		offscreen[0xffff & (di+256)] = color;
		offscreen[0xffff & (di+257)] = color;
		offscreen[0xffff & (si+256)] = color;
		offscreen[0xffff & (si+257)] = color;
		di+=0x202;
		si-=0x1fe;
	}
	if(len!=1)
	{
		di-=0x400;
		si+=0x400;
		for(i=1;i<len;++i)
		{
			offscreen[0xffff & (di+0)] = color;
			offscreen[0xffff & (di+1)] = color;
			offscreen[0xffff & (si+0)] = color;
			offscreen[0xffff & (si+1)] = color;
			offscreen[0xffff & (di+256)] = color;
			offscreen[0xffff & (di+257)] = color;
			offscreen[0xffff & (si+256)] = color;
			offscreen[0xffff & (si+257)] = color;
			di-=0x1fe;
			si+=0x202;
		}
	}
	loc += locd;
	len = len + lend;
	if(!len)
	{
		loc = 0x7e7e;
		len = 1;
		locd = -2;
		lend = 1;
		setnext();
	} else
	{
		if(len>=0x42)
		{
			loc=0xfe7e;
			len=0x41;
			locd=2;
			lend=-1;
		}
	}
}

void r_p50(void)
{
static unsigned char cntr=255;
int ebp;
int i;
int ebx;
int dl;
int edi;
int ah;

	edi = cntr<<8;
	ebp = cntr & 0x7f;
	if(ebp >= 0x40) ebp^=0x7f;
	ebp -= 0x20;
	dl=0;
	for(i=0;i<256;++i)
	{
		ebx=sincos0s[dl & 0xff] * ebp;
		dl+=8;
		ebx&=0xff00;
		ah = (ebx>>8) + 0xa0;
		offscreen[(edi+ebx    ) & 0xffff] = ah;
		offscreen[(edi+ebx+256) & 0xffff] = ah;
		++edi;
	}
	if(!cntr--)
		setnext();
}

void r_p60(void)
{
static unsigned char loc=0;
	memset(offscreen + loc*256, 0, 0x2000);
	loc += 0x20;
	if(!loc)
		setnext();
}

int r_p70r0(int dest, int sin, int ch)
{
int i,j,k;
int cl;
int di;

	for(i=0;i<4;++i)
	{
		cl=sincos0[sin & 0xff];
		sin+=4;
		cl>>=3;
		di = dest + (ch*256) + cl;
		for(j=0;j<15;++j)
		{
			for(k=0;k<16;++k)
				offscreen[(di+k)&0xffff] += 4;
			di+=0x100;
		}
		dest += ch*256;
	}
	return (dest>>8) & 0xff;
}
void r_p70(void)
{
static unsigned char loc[2]={0,0};
static unsigned char sinloc=0;
	loc[0] = r_p70r0(loc[0]*256       , sinloc,  1);
	loc[1] = r_p70r0(loc[1]*256 + 0x80, sinloc, -1);
	sinloc+=16;
	if(!loc[1])
		setnext();
}

void r_p80(void)
{
static unsigned short loc=0xe000;
static unsigned short locd=0x2000;
static unsigned short locdtbl[]={
	0x2000, 0x20, 0xe000, 0xffe0, 0x2000, 0x20, 0xe000,
	0xffe0, 0x2000, 0x20, 0xe000, 0xffe0, 0x2000, 0x20, 0xe000, 0};
static unsigned char cntr=8;
static unsigned char index=0;
static unsigned char cntrtbl[]={8,7,7,6,6,5,5, 4,4,3,3,2,2,1, 1, 1};
int i;

	loc+=locd;
	if(!--cntr)
	{
		index=(index+1)&15;
		cntr=cntrtbl[index];
		locd=locdtbl[index];
		if(!index)
		{
			loc=0;
			setnext();
		}
	}
	for(i=0;i<32;++i)
		memcpy(offscreen+((loc+i*256)&0xffff), d8tile+i*32, 32);
}

void r_p90(void)
{
static unsigned char cntr=0;
int delta,color;
unsigned char *p;
int i;

	delta=1;
	color = cntr & 0x7f;
	if(color&0x40) {color^=0x7f;delta=-1;}
	p=offscreen + cntr*256;
	for(i=0;i<4;++i)
	{
		memset(p, color, 256);
		p+=256;
		color+=delta;
	}
	if(!(cntr+=4))
		setnext();
}


// scale factors = 32 = 1:1 scale, 64 = 2:1 scale, 16 = 1:2 scale, etc.
void scaletile(unsigned short dest, unsigned char *src, int xscale, int yscale)
{
int x,y;
unsigned short dp;
int tx,ty;
int px,py;
unsigned char *sp, c;
	++xscale;
	++yscale;
	px = (32<<16) / xscale;
	py = (32<<16) / yscale;

	ty = py>>1;
	for(y=0;y<yscale;++y)
	{
		dp = dest+y*256;
		sp = src + 32*(ty>>16);
		tx = px>>1;
		for(x=0;x<xscale;++x)
		{
			dest&=0xffff;
			c=sp[tx>>16];
			if(c)
				offscreen[dp] = c;
			++dp;
			tx += px;
		}
		ty += py;
	}
}

// from l0p3.m growing 4 bitmaps at rnd locs
void r_p30(void)
{
static unsigned short d3loc=0x53c7;
static unsigned short d3locd=0x137e;
static unsigned char d3size=0;
static unsigned char d3counter2=4;
static unsigned char d3sizetbl[]={4,4,4,4, 5,5,6,7};
static unsigned int d3sizeseed=0x974adec2;
int size, scale;
int t;

	size = 255 & (1<<d3size++);
	scale = size-1;

	scaletile(d3loc-size*256 - size, d3tile       , scale, scale);
	scaletile(d3loc-size*256       , d3tile + 1024, scale, scale);
	scaletile(d3loc          - size, d3tile + 2048, scale, scale);
	scaletile(d3loc                , d3tile + 3072, scale, scale);

	if(!--d3counter2)
	{
		d3locd += 0x4377;
		d3locd = (d3locd<<5) | (d3locd>>11);
		d3loc = t = d3loc + d3locd + (d3locd & 1);
		d3sizeseed += d3loc;
		if(t&0x10000) ++d3sizeseed;
		d3sizeseed = (d3sizeseed<<9) | (d3sizeseed>>23);
		d3counter2 = d3sizetbl[d3sizeseed & 7];
		d3size = 0;
		if(!--d3counter) setnext();

/*
		t=(d3locd + 0x4377);
		t = (t<<5) | ((t&0xffff)>>11);
		d3locd = t;
		d3loc += t;
		if(t&0x10000)
			++d3loc;
		if(t>0xffff)
			t=(t&0xffff) + d3sizeseed+1;
		else
			t+=d3sizeseed;
		t=((t&0x7ff) << 5) | ((t&0xf800)>>11);
		d3sizeseed=t;
		d3counter2=d3sizetbl[t&7];
		d3size=0;
		if(!--d3counter) setnext();
*/
	}
}


/*****************************************************
 *****************************************************
 Graphics routines, l1 stuff
 ****************************************************
 ****************************************************/

void r_l1p00(void);
void r_l1p10(void);
void r_l1p20(void);
void r_l1p30(void);
void r_l1p40(void);
void r_l1p50(void);
void r_l1p60(void);
void r_l1p70(void);
void r_l1p80(void);
void r_l1p90(void);
void r_l1restart(void);
void l1setnext(void);

void (*l1routtbl[])(void)={
 r_l1p00, r_l1p10, r_l1p20, r_l1p30, r_l1p40,
 r_l1p60, r_l1p70, r_l1p80, r_l1p50, r_l1p90,
 r_l1p00, r_l1p20, r_l1p30, r_l1p40, r_l1p60,
 r_l1p50, r_l1p10, r_l1p80, r_l1p70, r_l1p90,
 r_l1p30, r_l1p40, r_l1p50, r_l1p60, r_l1p00,
 r_l1p80, r_l1p10, r_l1p20, r_l1p90, r_l1p70,
 r_l1restart
};

int l1index=0;

void r_l1restart(void)
{
	l1index=0;
	l1setnext();
}

void l1setnext(void)
{
	l1rout = l1routtbl[l1index++];
}


// from l1p0.m, rolling daisy
void r_l1p00(void)
{
static unsigned char *csprtbl[] =
	{cspr02,cspr01,cspr00};
static int csprloctbl[]={400*168+320, 400*200, -32, 400*-32+288};
static short csprdtbl[]={-4, -1600, 4, 1600};
static unsigned char csprcntrtbl[]={88,58,88,58};
static int csprloc=400*168+320;
static short csprd=-4;
static unsigned char csprindex=0;
static unsigned char csprcntr=88;
static unsigned char csprnexti=0;
	csprloc += csprd;
	if(!csprindex--) csprindex=2;
	drawsprite(onscreen + csprloc, csprtbl[csprindex]);
	if(!--csprcntr)
	{
		csprnexti = (csprnexti+1)&3;
		csprloc = csprloctbl[csprnexti];
		csprd = csprdtbl[csprnexti];
		csprcntr = csprcntrtbl[csprnexti];
		l1setnext();
	}
}

// from l1p1.m, fishies
void r_l1p10(void)
{
static unsigned char *fishframetbl[] =
	{cspr03,cspr04};
static unsigned int fishseed=0x974adec2;
static unsigned short fishx[]={0,0,0,0,0,0,0,0,0,0};
static unsigned short fishy[]={0,0,0,0,0,0,0,0,0,0};
static unsigned char fishframe[]={0,0,0,0,0,0,0,0,0,0};
static int fishyaddtbl[]={-400,0,0,400};
static unsigned char fishendcntr=10;
static unsigned char fishinitcntr=10;
static unsigned char fishinitcntr2=1;
int i,t;
unsigned char *dest=onscreen - 5*4, *edi;
unsigned int ecx;
int edx;
	ecx = fishseed;
	for(i=9;i>=0;ecx+=i,--i)
	{
		t=fishx[i]-3;
		if(t>=0)
		{
			fishx[i]=t;
			if(t==0)
			{
				if(!--fishendcntr)
				{
					fishendcntr=10;
					fishinitcntr=10;
					l1setnext();
					return;
				}
				continue;
			}
			fishframe[i]^=1;
			edi = dest + t;
			edx = fishy[i] + fishyaddtbl[ecx&3];
			if(edx>=0 && edx<=183*400)
				fishy[i] = edx;
			edi += edx;
			drawsprite(edi, fishframetbl[fishframe[i]]);
			ecx = (ecx>>11) | (ecx<<21);
			if(ecx & (1<<31))
				++ecx;
			ecx += 0x12345671;
		}
	}
	if(fishinitcntr && !--fishinitcntr2)
	{
		--fishinitcntr;
		fishinitcntr2=(ecx&15) + 4;
		fishx[fishinitcntr]=342;
		fishy[fishinitcntr]=(((ecx>>8)&0x7f)+27)*400;
		ecx = (ecx>>5) | (ecx<<27);
		if(ecx & (1<<31))
			++ecx;
		ecx += 0x59f4132f;
	}
	fishseed = ecx;

}

// from l1p2.m, slug
void r_l1p20(void)
{
static unsigned char *sluggycsprtbl[] =
	{cspr06,cspr07,cspr08,cspr07,cspr06,cspr05};
static unsigned char sluggyframe=5;
static int sluggyy=233;
static unsigned char sluggyx=0x71;
static unsigned char sluggyofftbl[]={9,5,0,0,0,0};
unsigned char *dest;

	dest=onscreen + (sluggyy-sluggyofftbl[sluggyframe]) * ON_W + sluggyx;
	dest += -29*ON_W + 6;
	drawsprite(dest, sluggycsprtbl[sluggyframe]);

	if(!sluggyframe--)
	{
		sluggyframe=5;
		if(sluggyy>=13)
			sluggyy-=13;
		else
		{
			sluggyy=233;
			sluggyx+=0x6f;
			l1setnext();
		}
	}
}

// from l1p3.m, happy faces
void r_l1p30(void)
{
static unsigned int happyseed=0x9693972e;
static unsigned short happyloc[16]={0};
static unsigned char happyindex=0;
static short happymloc=-40;
int i;
int eax;
int ecx;

	for(i=15;i>=0;--i)
		drawsprite(onscreen + happyloc[i], cspr09);

	happyindex=(happyindex+1)&15;
	eax = (happyseed & 0x3f) + happymloc;
	if(eax<0 || eax>=320) eax=320;
	ecx = (happyseed >> 6) & 255;
	if(ecx >= 200-12) ecx -= 156;
	happyloc[happyindex] = ecx*400 + eax;
	happyseed = (happyseed >> 9) | (happyseed << 23);
	happyseed += 0x123457 + happyindex;
	happymloc += 7;
	if(happymloc >= 320+7*16)
	{
		happymloc = -40;
		l1setnext();
	}
}

// from l1p4.m
int p40r0a(unsigned short *loc, int *x, int *y)
{
	*x=*y=0;
	*loc += 2;
	if(*loc>=50)
	{
		*x = 131+(sincos0[*loc & 255]>>1);
		*y = *loc - 50;
	}
	return 0;
}
int p40r0b(unsigned short *loc, int *x, int *y)
{
	*x=*y=0;
	if(*loc==276) return 1;
	if(*loc >= 226) return 0;

	*x = 131 + sincos0[(*loc - 50 + 0x80)&0xff];
	*y = *loc;
	return 0;
}

int p40r1a(unsigned short *loc, int *x, int *y)
{
	*x=*y=0;
	*loc += 3;
	if(*loc < 80) return 0;

	*x = *loc - 80;
	*y = 49+(sincos0[*x & 0xff]>>1);
	return 0;
}
int p40r1b(unsigned short *loc, int *x, int *y)
{
	*x=*y=0;
	if(*loc >= 472) return 1;
	if(*loc >= 392) return 0;
	*x = *loc;
	*y = 49 + (sincos0[(*x - 80 - 0x80)&0xff]>>1);
	return 0;
}

int p40r2a(unsigned short *loc, int *x, int *y)
{
	*x=*y=0;
	*loc += 2;
	if(*loc >= 50)
	{
		*y = *loc - 50;
		*x = 131 + (sincos0[(*y - 50 - 0x80)&0xff]>>1);
		*y = 226 - *y;
	}

	return 0;
}
int p40r2b(unsigned short *loc, int *x, int *y)
{
	*x=*y=0;
	if(*loc == 276) return 1;
	if(*loc >= 226) return 0;
	*x = 131 + (sincos0[(*loc - 50 - 0x80)&0xff]>>1);
	*y = 224 - *loc;
	return 0;
}
int p40r3a(unsigned short *loc, int *x, int *y)
{
	*x=*y=0;
	*loc += 3;
	if(*loc < 78) return 0;
	*x = *loc - 78;
	*y = 49 + (sincos0[*x & 255]>>1);
	*x = 396 - *x;

	return 0;
}
int p40r3b(unsigned short *loc, int *x, int *y)
{
	*x=*y=0;
	if(*loc == 474) return 1;
	if(*loc >= 396) return 0;
	*y = 49 + (sincos0[(*loc - 78 - 0x80)&0xff]>>1);
	*x = 393 - *loc;
	return 0;
}

void r_l1p40(void)
{
static int (*posrouttbl[2][8])(unsigned short *loc, int *x, int *y)={
	{p40r0a, p40r1a, p40r2a, p40r3a, p40r0a, p40r1a, p40r2a, p40r3a },
	{p40r0b, p40r1b, p40r2b, p40r3b, p40r0b, p40r1b, p40r2b, p40r3b }};
static unsigned char *wingframe[]=
	{cspr0b,cspr0c,cspr0d,cspr0c};
static unsigned char *centercsprtbl[]={cspr0a,cspr17,cspr11,cspr10,cspr22};
static unsigned short loc;
static unsigned char frame[2]={0,2};
static unsigned char posroutindex=0;
static unsigned char centercspri=0;
int i, res, x=160, y=100;
unsigned char *dest;

	for(i=0;i<2;++i)
	{
		res=posrouttbl[i][posroutindex](&loc, &x, &y);
		frame[i] = (frame[i]+1)&3;
		dest = onscreen + (y-26)*ON_W + x-18*4;
		drawsprite(dest, wingframe[frame[i]]);
		drawsprite(dest + 9*ON_W + 6*4, centercsprtbl[centercspri]);
		if(i==1 && res)
		{
			posroutindex = (posroutindex+1) & 7;
			loc = 0;
			++centercspri;
			if(centercspri == 5)
				centercspri = 0;
			l1setnext();
		}
	}
}

// from l1p5.m
void r_l1p50(void)
{
static unsigned char *cspr=cspr0e;
static unsigned char *csprtbl[]=
	{cspr0e,cspr0f,cspr12,cspr13,cspr14,cspr15};
static unsigned short csprloc=372;
static unsigned char csprindex=0;
static unsigned char heighttbl[]={
	37,36,36,35,34,33,33,32,31,30,30,29,28,28,27,26,
	26,25,24,24,23,23,22,21,21,20,20,19,18,18,17,17,
	16,16,15,15,14,14,13,13,12,12,11,11,10,10,10,9,
	9,8,8,8,7,7,6,6,6,5,5,5,5,4,4,4,
	3,3,3,3,2,2,2,2,2,1,1,1,1,1,1,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
	1,1,2,2,2,2,2,3,3,3,3,4,4,4,5,5,
	5,5,6,6,6,7,7,8,8,8,9,9,10,10,10,11,
	11,12,12,13,13,14,14,15,15,16,16,17,17,18,18,19,
	20,20,21,21,22,23,23,24,24,25,26,26,27,28,28,29,
	30,30,31,32,33,33,34,35,36,36,37
};

	csprloc-=2;
	if(!csprloc)
	{
		++csprindex;
		if(csprindex==6)
			csprindex=0;
		cspr = csprtbl[csprindex];
		csprloc = 372;
		l1setnext();
	}
	drawsprite(onscreen + heighttbl[csprloc>>1]*400 + csprloc + 15*400 - 13*4,
		 cspr);
}

// from l1p6.m
void r_l1p60(void)
{
static int locs0[64*2];
static int locs1[32*2];
static int locs2[16*2];
static int first=1;
static unsigned char cntr = 0xf0;
static unsigned char num=0;
static unsigned char numd=1;
int temp0, temp1, temp2, temp3;
int i;
int *p;
int ah;
int t;
	if(first)
	{
		temp0=0;
		temp1=79;
		temp2=0;
		temp3=67*4+1;
		p=locs0;
		for(i=0;i<64;++i)
		{
			*p++ = temp0; // y
			*p++ = temp2; // x
			temp0 = temp0 + temp1;
			if(temp0 > 199)
			{
				temp0 = temp0 - 200;
				temp1 = temp1 - 3;
			}
			temp2 = temp2 + temp3;
			if(temp2 > 319)
			{
				temp2 = temp2 - 320;
				temp3 = temp3 - 16;
			}
		}

		temp0=56;
		temp1=95;
		temp2=49*4;
		temp3=59*4+2;
		p=locs1;
		for(i=0;i<32;++i)
		{
			*p++ = temp0; // y
			*p++ = temp2; // x
			temp0 = temp0 + temp1;
			if(temp0 > 199)
			{
				temp0 = temp0 - 200;
				temp1 = temp1 - 6;
			}
			temp2 = temp2 + temp3;
			if(temp2 > 319)
			{
				temp2 = temp2 - 320;
				temp3 = temp3 - 20;
			}
		}

		temp0=132;
		temp1=96;
		temp2=13*4;
		temp3=73*4;
		p=locs2;
		for(i=0;i<16;++i)
		{
			*p++ = temp0; // y
			*p++ = temp2; // x
			temp0 = temp0 + temp1;
			if(temp0 > 199)
			{
				temp0 = temp0 - 200;
				temp1 = temp1 - 8;
			}
			temp2 = temp2 + temp3;
			if(temp2 > 319)
			{
				temp2 = temp2 - 320;
				temp3 = temp3 - 7*4;
			}
		}
		first = 0;
	}

	if(!--cntr)
	{
		cntr = 0xf0;
		num = 0;
		numd = 1;
		l1setnext();
		return;
	}
	if(cntr == 0x10)
		numd = -1;
	num += numd;
	if(num==0 || num>=0x10)
		numd=0;

	ah = 0xc7;
	for(i=0;i<16;++i)
	{
		p = locs2 + i*2;
		*p += 4;
		if(*p >= 200)
			*p -= 200;
		p[1]-=4;
		if(p[1]<0)
			p[1] += 320;
		if(i<num)
		{
			t = p[0]*ON_W + p[1];
			memset(onscreen+t+1, ah, 2);
			memset(onscreen+ON_W*3+t+1, ah, 2);
			memset(onscreen+t+ON_W, ah, 4);
			memset(onscreen+t+ON_W*2, ah, 4);
		}
	}
	for(i=0;i<32;++i)
	{
		p = locs1 + i*2;
		*p += 2;
		if(*p >= 200)
			*p -= 200;
		p[1]-=2;
		if(p[1]<0)
			p[1] += 320;
		if(i<num*2)
		{
			t = p[0]*ON_W + p[1];
			onscreen[t]=ah;
			onscreen[t+1]=ah;
			onscreen[t+ON_W]=ah;
			onscreen[t+1+ON_W]=ah;
		}
	}
	for(i=0;i<64;++i)
	{
		p = locs0 + i*2;
		(*p)++;
		if(*p >= 200)
			*p -= 200;
		--p[1];
		if(p[1]<0)
			p[1]+=320;
		if(i<num*4)
			onscreen[p[0]*ON_W + p[1]] = ah;
	}
}

// from l1p7.m
void r_l1p70(void)
{
static unsigned char *csprtbl[3]={cspr16,cspr0a,cspr17};
static unsigned char eccx=0x40, eccy=0x40;
static unsigned char eccdx = 0xff, eccdy = 0xff;
static unsigned char cspri = 0;
static unsigned char ecccntr = 3;
static int x=0;
static int xd = 2;
static unsigned char phase = 0;
unsigned char *d;
int i;
int tx, ty;
	x += xd;
	if(x==0 || x>=470)
	{
		xd = -xd;
		++cspri;
		if(cspri == 2)
			cspri = 0;
		l1setnext();
		return;
	}
	eccy += eccdy;
	if(eccy < 0x10 ||  eccy >= 0x40)
		eccdy = -eccdy;
	if(!--ecccntr)
	{
		ecccntr = 3;
		eccx += eccdx;
		if(eccx < 0x10 || eccx >= 0x40)
			eccdx = -eccdx;
	}
	d = onscreen + (0x40 - eccy) * ON_W;
	phase+=5;
	for(i=0;i<8;++i)
	{
		ty = (sincos0[(phase + i*32) & 0xff] * eccy) >> 7;
		tx = x + ((sincos0[(phase + i*32 + 0x40) & 0xff] * eccx) >> 7);
		tx -= 128;
		if(tx<0 || tx>=340) continue;
		drawsprite(d + (ty+26)*ON_W + tx - 20, csprtbl[cspri]);
	}


}

// from l1p8.m
void r_l1p80(void)
{
static int locs0[64*2];
static int locs1[32*2];
static int locs2[16*2];
static int first=1;
static unsigned char cntr=0xf0;
static unsigned char num=0, numd=1;
unsigned char ah;

int *p,i,j;
int t;

	if(first)
	{
		int temp0, temp1, temp2, temp3;
		temp0=192;
		temp1=188;
		temp2=53*4;
		temp3=37*4;
		p=locs0;
		for(i=0;i<64;++i)
		{
			*p++ = temp0; // y
			*p++ = temp2; // x
			temp0 = temp0 + temp1;
			if(temp0 > 199)
			{
				temp0 = temp0 - 200;
				temp1 = temp1 - 8;
			}
			temp2 = temp2 + temp3;
			if(temp2 > 319)
			{
				temp2 = temp2 - 320;
				temp3 = temp3 - 7*4;
			}
		}

		temp0=64;
		temp1=96;
		temp2=25*4;
		temp3=31*4+2;
		p=locs1;
		for(i=0;i<32;++i)
		{
			*p++ = temp0; // y
			*p++ = temp2*2; // x
			temp0 = temp0 + temp1;
			if(temp0 > 199)
			{
				temp0 = temp0 - 200;
				temp1 = temp1 - 8;
			}
			temp2 = temp2 + temp3;
			if(temp2 > 159)
			{
				temp2 = temp2 - 160;
				temp3 = temp3 - 3*4;
			}
		}

		temp0=132;
		temp1=96;
		temp2=13*4;
		temp3=13*4;
		p=locs2;
		for(i=0;i<16;++i)
		{
			*p++ = temp0; // y
			*p++ = temp2*4; // x
			temp0 = temp0 + temp1;
			if(temp0 > 199)
			{
				temp0 = temp0 - 200;
				temp1 = temp1 - 8;
			}
			temp2 = temp2 + temp3;
			if(temp2 > 79)
			{
				temp2 = temp2 - 80;
				temp3 = temp3 - 2*4;
			}
		}
		first = 0;
	}
	if(!--cntr)
	{
		cntr = 0xf0;
		num=0;
		numd=1;
		l1setnext();
		return;
	}
	if(cntr == 0x10)
		numd = -1;
	num += numd;
	if(num==0 || num==0x10)
		numd = 0;
	ah = 0xd9;
	for(i=0;i<16;++i)
	{
		p=locs2 + i*2;
		p[0] += 16;
		if(p[0]>=208)
			p[0] -= 200;
		p[1] -= 16;
		if(p[1]<0)
			p[1] += 320;
		if(i<num)
		{
			t = p[0]*ON_W + p[1];
			for(j=0;j<16;++j)
				onscreen[t - j*(ON_W - 1)] = ah;
		}
	}

	for(i=0;i<32;++i)
	{
		p=locs1 + i*2;
		p[0] += 8;
		if(p[0] >= 204)
			p[0] -= 200;
		p[1] -= 8;
		if(p[1]<0)
			p[1] += 320;
		if(i<num*2)
		{
			t = p[0]*ON_W + p[1];
			for(j=0;j<8;++j)
				onscreen[t - j*(ON_W - 1)] = ah;
		}
	}

	for(i=0;i<32;++i)
	{
		p=locs0 + i*2;
		p[0] += 4;
		if(p[0] >= 202)
			p[0] -= 200;
		p[1] -= 4;
		if(p[1]<0)
			p[1] += 320;
		if(i<num*4)
		{
			t = p[0]*ON_W + p[1];
			for(j=0;j<4;++j)
				onscreen[t - j*(ON_W - 1)] = ah;
		}
	}
}

// from l1p9.m
void r_l1p90(void)
{
static unsigned char *frametbl[]={
	cspr18,cspr19,cspr19,cspr1a,cspr1a,cspr1b,cspr1b,cspr1c,
	cspr1d,cspr1e,cspr1e,cspr1f,cspr1f,cspr20,cspr20,cspr21};
static unsigned short leafx[24];
static unsigned short leafy[24];
static unsigned char dphase[2]={0,0};
static unsigned char cntr=0xf0;
static unsigned char num=0;
static unsigned char numd=1;

static int first=1;
int i;

	if(first)
	{
		int temp0, temp1;
		temp0 = 152;
		temp1 = 187;
		for(i=0;i<24;++i)
		{
			leafx[i] = temp0;
			temp0 += temp1;
			if(temp0 > 333)
			{
				temp0 -= 334;
				temp1 -= 17;
			}
		}

		temp0 = 137;
		temp1 = 167;
		for(i=0;i<24;++i)
		{
			leafy[i] = temp0;
			temp0 += temp1;
			if(temp0 > 209)
			{
				temp0 -= 210;
				temp1 -= 11;
			}
		}

		first = 0;
	}

	if(!--cntr)
	{
		cntr = 0xf0;
		num = 0;
		numd = 1;
		for(i=0;i<8;++i)
		{
			unsigned char *tp;
			tp = frametbl[i];
			frametbl[i] = frametbl[i+8];
			frametbl[i+8] = tp;
		}
		l1setnext();
		return;
	}
	if(cntr == 23)
		numd = -1;
	num += numd;
	if(!num || num==23)
		numd = 0;
	dphase[0] += 7;
	dphase[1] += 13;

	for(i=num;i>=0;--i)
	{
		int t, tx, ty;

		ty = (sincos0[(dphase[0] + leafy[i])&0xff]>>6) + 1;
		ty = leafy[i] + ty;
		if(ty >= 210) ty-=210;
		leafy[i] = ty;

		t = sincos0[(dphase[1] + ty)&0xff]>>5;
		tx = (t>>1) + 1;
		tx = leafx[i] - tx;
		if(tx < 0) tx += 334;
		leafx[i] = tx;
		drawsprite(onscreen + (ty-9)*ON_W + tx - 13, frametbl[t]);
	}



}


void l1start(void)
{
	if(l1on)
		l1setnext();
}


/*****************************************************
 *****************************************************
 Graphics routines, l2 stuff
 ****************************************************
 ****************************************************/

void l2start(void)
{
}


/*****************************************************
 *****************************************************
 Palette related helper functions
 ****************************************************
 ****************************************************/

unsigned char thepal[256*3];
unsigned char wantpal[256*3];

#define COLOROUT(r,g,b) {*p++=r; *p++=g; *p++=b;}
void palinit(void)
{
unsigned char *p;
int temp,i;
	p=pal00; // white
	for(i=temp=0;i<64;++i)
		COLOROUT(temp, temp, temp++)
	for(i=0;i<63;++i)
		COLOROUT(--temp, temp, temp)

	p=pal01; // blue
	for(i=temp=0;i<64;++i)
		COLOROUT(0, 0, temp++)
	--temp;
	for(i=0;i<63;++i)
		COLOROUT(0, 0, --temp)


	p=pal02; // green
	for(i=temp=0;i<64;++i)
		COLOROUT(0, temp++, 0)
	--temp;
	for(i=0;i<63;++i)
		COLOROUT(0, --temp, 0)

	p=pal03; // red
	for(i=temp=0;i<64;++i)
		COLOROUT(temp++, 0, 0)
	--temp;
	for(i=0;i<63;++i)
		COLOROUT(--temp, 0, 0)

	p=pal04; // light blue
	for(i=temp=0;i<64;++i)
		COLOROUT(0, temp/2, temp++)
	--temp;
	for(i=0;i<63;++i)
		COLOROUT(0, --temp/2, temp)

	p=pal05; // light green
	for(i=temp=0;i<64;++i)
		COLOROUT(temp/2, temp++, 0)
	--temp;
	for(i=0;i<63;++i)
		COLOROUT(--temp/2, temp, 0)

	p=pal06; // magenta
	for(i=temp=0;i<64;++i)
		COLOROUT(temp, 0, temp++/2)
	--temp;
	for(i=0;i<63;++i)
		COLOROUT(--temp, 0, temp/2)

	p=pal07; // blue/purple
	for(i=temp=0;i<32;++i)
		COLOROUT(0, 0, temp++*2)
	for(i=temp=0;i<32;++i)
		COLOROUT(temp++*2, 0, 63)
	--temp;
	for(i=0;i<31;++i)
		COLOROUT(--temp*2, 0, 63)
	for(i=0,temp=32;i<32;++i)
		COLOROUT(0, 0, --temp*2)

	p=pal08; // green/light cyan
	for(i=temp=0;i<32;++i)
		COLOROUT(0, temp++*2, 0)
	for(i=temp=0;i<32;++i)
		COLOROUT(0, 63, temp++*2)
	--temp;
	for(i=0;i<31;++i)
		COLOROUT(0, 63, --temp*2)
	for(i=0,temp=32;i<32;++i)
		COLOROUT(0, --temp*2, 0)

	p=pal09; // red/yellow
	for(i=temp=0;i<32;++i)
		COLOROUT(temp++*2, 0, 0)
	for(i=temp=0;i<32;++i)
		COLOROUT(63, temp++*2, 0)
	--temp;
	for(i=0;i<31;++i)
		COLOROUT(63, --temp*2, 0)
	for(i=0,temp=32;i<32;++i)
		COLOROUT(--temp*2, 0, 0)

	p=pal0b; // violet
	for(i=temp=0;i<64;++i)
		COLOROUT(temp/2, 0, temp++)
	--temp;
	for(i=0;i<63;++i)
		COLOROUT(--temp/2, 0, temp)

	p=pal0c; // blue green
	for(i=temp=0;i<64;++i)
		COLOROUT(0, temp, temp++/2)
	--temp;
	for(i=0;i<63;++i)
		COLOROUT(0, --temp, temp/2)

	p=pal0d; // orange
	for(i=temp=0;i<64;++i)
		COLOROUT(temp, temp++/2, 0)
	--temp;
	for(i=0;i<63;++i)
		COLOROUT(--temp, temp/2, 0)

	p=pal0e; // blue/white
	for(i=temp=0;i<32;++i)
		COLOROUT(0, 0, temp++*2)
	for(i=temp=0;i<32;++i)
		COLOROUT(temp*2, temp++*2, 63)
	--temp;
	for(i=0;i<31;++i)
		COLOROUT(--temp*2, temp*2, 63)
	for(i=0,temp=32;i<32;++i)
		COLOROUT(0, 0, --temp*2)

	p=pal0f; // green/white
	for(i=temp=0;i<32;++i)
		COLOROUT(0, temp++*2, 0)
	for(i=temp=0;i<32;++i)
		COLOROUT(temp*2, 63, temp++*2)
	--temp;
	for(i=0;i<31;++i)
		COLOROUT(--temp*2, 63, temp*2)
	for(i=0,temp=32;i<32;++i)
		COLOROUT(0, --temp*2, 0)

	p=pal10; // red/white
	for(i=temp=0;i<32;++i)
		COLOROUT(temp++*2, 0, 0)
	for(i=temp=0;i<32;++i)
		COLOROUT(63, temp*2, temp++*2)
	--temp;
	for(i=0;i<31;++i)
		COLOROUT(63, --temp*2, temp*2)
	for(i=0,temp=32;i<32;++i)
		COLOROUT(--temp*2, 0, 0)

//	memcpy(thepal+192*3, pal0aptr, 64*3);
}

int palindex=0;

unsigned char *palptrtbl[]={
	pal02, pal00,pal01,pal06,pal09,pal01,pal07,pal04,pal03,
	pal05,pal0e,pal0d,pal0c,pal0b,pal03,
	pal01,pal0f,pal10,pal04,pal08,pal0b
};

#define PALTBLLEN (sizeof(palptrtbl)/sizeof(palptrtbl[0]))

int slidewhich=-1;
int slidelen;
unsigned char *slidetake;

void doslide(void)
{
unsigned char *s, *d;
int i,j;
int delta;

	if(slidewhich>=0)
	{
		delta = 1;
		s=slidetake;
		d=thepal+slidewhich*3;
		for(i=j=0;i<slidelen*3;++i)
		{
			if(*s<*d)
			{
				if(*d -1 == *s)
					--*d;
				else
					*d -= delta;
				++j;
			}
			else if(*s>*d)
			{
				if(*d +1 == *s)
					++*d;
				else
					*d += delta;
				++j;
			}
			++s;
			++d;
		}
		if(!j)
			slidewhich=-1;
	}
}

void waitpals(void)
{
	if(slidewhich<0)
	{
		setnext();
	}
}

void nextpal(int n, int slide)
{
unsigned char *put;
	if(++palindex == PALTBLLEN)
		palindex=0;
//printf("nextpal %d, index %d\n", n, palindex);
	put = slide ? wantpal : thepal;
	memcpy(put+n*64*3, palptrtbl[palindex], 64*3);
	if(slide)
	{
		slidewhich=n*64;
		slidelen=64;
		slidetake = wantpal + n*64*3;
	}
}

void set_pals(int n, int len, unsigned char *pal)
{
	slidelen=len;
	slidewhich=n;
	slidetake = pal;
}

void set_pal(int n, int len, unsigned char *pal)
{
	memcpy(thepal + n*3, pal, len*3);
}

/*****************************************************
 *****************************************************
 Palette related routines
 ****************************************************
 ****************************************************/
void r_m00(void)
{
	nextpal(0,1);
	setnext();
}

void r_m01(void)
{
	nextpal(1,1);
	setnext();
}

void r_m02(void)
{
	set_pals(0,64,_bpal);
	setnext();
}

void r_m03(void)
{
	nextpal(2,1);
	setnext();
}

void r_m04(void)
{
	set_pals(192,64, pal0aptr);
	nextpal(1,0);
	setnext();
}

void r_m05(void)
{
	nextpal(2,1);
	d3tile = gradient_sphere;
	d3counter = 0x10;
	delaycntr = 2;
	setnext();
}

void r_m06(void)
{
	l1on=1;
	set_pals(128, 64, _bpal);
	d3counter = 0x20;
	setnext();
}

void r_m07(void)
{
	d3tile = daisy;
	d3counter = 0x40;
	setnext();
}

void r_m08(void)
{
	nextpal(1,1);
	delaycntr=0x80;
	setnext();
}

void r_m09(void)
{
#warning turn on _l2on
// **** TURN ON _l2on
	nextpal(2,0);
	nextpal(1,1);
	setnext();
}

void r_m0a(void)
{
	d3tile = lightning;
	d3counter = 0x10;
	delaycntr = 0x40;
	setnext();
}

void r_m0b(void)
{
	delaycntr = 0x40;
	setnext();
}

void r_m0c(void)
{
	set_pals(0,192,_bpal);
	setnext();
}

void r_m0d(void)
{
	nextpal(0,0);
	delaycntr = 0x40;
	setnext();
}

void r_m0e(void)
{
	nextpal(1,0);
	d3tile = star_thingy;
	d3counter = 0x10;
	delaycntr = 0x40;
	setnext();
}

void r_m0f(void)
{
	nextpal(2,0);
	d8tile = small_gradient_sphere;
	delaycntr=0x40;
	setnext();
}

void r_m10(void)
{
	d3tile=eyeball;
	d3counter=0x10;
	setnext();
}

void r_m11(void)
{
	nextpal(0,0);
	set_pals(128,64,_bpal);
	d3counter=0x40;
	delaycntr=0x40;
	setnext();
}

void r_m12(void)
{
	set_pals(64, 64, palptrtbl[palindex]+63*3);
	setnext();
}

void r_m13(void)
{
	nextpal(2,0);
	setnext();
}

void r_m14(void)
{
	set_pal(0, 64, _bpal);
	set_pals(128, 64, _bpal);
	setnext();
}

void r_m15(void)
{
	nextpal(0,1);
	d3tile = face;
	d3counter=0x20;
	delaycntr=0x80;
	setnext();
}

void r_m16(void)
{
	d3tile = sundial;
	d3counter=0x10;
	delaycntr=0x40;
	setnext();
}

struct mover {
	int at;
	int moving;
	int delaycount;
	int delayperiod;
	int which;
	int wants[8];
};
int stepmover(struct mover *m)
{
	if(!m->moving)
	{
		if(!--m->delaycount)
		{
			m->which=(m->which+1) & 7;
			m->delaycount = m->delayperiod;
			m->moving=1;
		}
	} else
	{
		int want=m->wants[m->which];
		if(m->at < want)
			++m->at;
		else if(m->at > want)
			--m->at;
		else
			m->moving=0;
	}
	return m->at;
}

/*****************************************************
 *****************************************************
 Generate the bg scan table algorithmically instead of as data
 ****************************************************
 ****************************************************/

void init_gnbgtbl(void)
{
int i,j;
double a;
unsigned char *p;
double f;
double rtab[256];

	for(i=0;i<100;++i)
	{
		f=i;
		rtab[i] = 256*99.0 / (16*99 - 15*f); // 3d projection approach
//		rtab[i] = (15.8891 + .0512321*f)/(1.0-.00926684*f); // Approx. Tran's
	}

	p=gnbgtbl;
	for(i=0;i<320;++i)
	{
#define PI 3.14159265358979323846264338327950288
		a = i*(PI/160.0);
		for(j=0;j<100;++j)
		{
			*p++ = rtab[j]*sin(a); // y
			*p++ = rtab[j]*cos(a); // x
		}
	}
	for(i=0;i<320;++i)
	{
		int atx, aty;
		int wx, wy;

		p=gnbgtbl + i*200;
		atx=aty=0;
		for(j=0;j<100;++j)
		{
			wy = p[0];
			wx = p[1];
			*p++ = wy-aty;
			*p++ = wx-atx;
			atx=wx;
			aty=wy;
		}
	}
}


/*****************************************************
 *****************************************************
 Draw the warped background
 ****************************************************
 ****************************************************/

void gnbg(void)
{
int x,y,c;
static unsigned char *skewptr=gnbgtbl;		// ptr to row to skew
static unsigned short skewcounter=0x10f;	// counter between skewing
static unsigned char skewmode=0;			// 0=wait, 1=skewing
static unsigned char skewindex=3;
static int skewtbl[]={1,-3,5,-3};
int eax;
int i;
static struct mover mtblvpdir={5,0,0xe7,0xf0,0,{5,-7,6,-5,9,-6,7,-9}};
static struct mover mtblvpy={3,0,0xc0,0xc0,0, {3,-1,5,-3,1,-5,1,-3}};
static struct mover mtblvpx={1,0,0x60,0xc0,0, {1,-5,1,-3,3,-1,5,-3}};

static int vpdir=0;		// viewpoint direction (0-319)
static unsigned short vploc=0x8080;	// viewpoint loc, Y*256+X
unsigned char *bp;

#if 1
	if(skewmode)
	{
		eax = skewtbl[skewindex];
		for(i=0;i<320;++i)
			skewptr[i*200] += eax;
		skewptr+=2;
		if(skewptr == gnbgtbl+200)
		{
			skewptr = gnbgtbl;
			skewmode=0;
		}

	} else if(!--skewcounter)
	{
		skewcounter=0x175;
		skewmode=1;
		skewindex=(skewindex+1) & 3;
	}

// do movement on X and Y
	i=stepmover(&mtblvpy);
	vploc += (i<<8);
	i=stepmover(&mtblvpx);
	vploc = (vploc&0xff00) | ((vploc+i)&0xff);
// do direction spin
	vpdir += stepmover(&mtblvpdir);
	if(vpdir<0) vpdir+=320;
	if(vpdir>=320) vpdir-=320;
#endif
	bp=gnbgtbl + 200*vpdir;
	for(x=0;x<320;++x)
	{
		unsigned char sx, sy;
		sx = vploc>>8;
		sy = vploc;

		for(y=0;y<100;++y)
		{
			sy += *bp++;
			sx += *bp++;
			c=offscreen[(sx<<8) | sy++];
			onscreen[x+y*ON_W]=c;
			onscreen[x+(199-y)*ON_W]=c;
		}
		if(bp==gnbgtbl + 64000)
			bp=gnbgtbl;
	}
}

void gnbg2(void)
{
int y;
unsigned char *p=offscreen;
unsigned char *d;
	for(y=0;y<256;++y)
	{
		d=onscreen+(y-28)*ON_W;
		memset(d, 0, 32);
		memcpy(d + 32, p+y*256, 256);
		memset(d + 32 + 256, 0, 32);
	}
}

void r_fast(void)
{
	fast=1;
	setnext();
}

void r_slow(void)
{
	fast=0;
	setnext();
}

struct scaleput {
	int x, y;
};

void copier(void *data, unsigned char *from, int len)
{
struct scaleput *sp = data;
	memcpy(thescreen->pixels+sp->y*thescreen->pitch + sp->x*4, from, len);
	++sp->y;
}

int interrupt;
Uint32 pulse(Uint32 interval, void *flagp)
{
	interrupt = 1;
	return interval;
}

void iterate(int flags)
{
int i;
unsigned char *p;
int x,y;
unsigned int map[256];
int n;
struct scaleput sp;
void *scaler;
int tx, ty;

	if(fast || (flags & OPTION_FAST))
		n = 8;
	else
		n = 1;
	if(flags & OPTION_PAUSED)
		n = 0;
	while(n--)
	{
		doslide();
		if(flags & OPTION_UNWARP_BG)
			gnbg2();
		else
			gnbg();
		l0rout();
		l1rout();
		l2rout();
	}

	p=thepal;
	for(i=0;i<256;++i)
	{
		map[i]=maprgb(p[0]*4, p[1]*4, p[2]*4);
		p+=3;
	}

	tx = ysize * 320/200;
	ty = xsize * 200/320;
//preserve aspect ratio
	if(tx>xsize) tx = xsize;
	if(ty>ysize) ty = ysize;
	sp.x = (xsize - tx)>>1;
	sp.y = (ysize - ty)>>1;
	scaler = scaler_alloc(tx, ty, 320, 200, 4, copier, &sp);
	scrlock();
	for(y=0;y<200;++y)
	{
		Uint32 line[320];
		p=onscreen + y*ON_W;
		for(x=0;x<320;++x)
			line[x] = map[*p++];
		scaler_feed(scaler, (unsigned char *)line);
	}
	scrunlock();
	scaler_free(scaler);
	update();
}

void general_init(void)
{
	init_gnbgtbl();
	palinit();
	l0rout = routtbl[0];
}

void resize(int w, int h)
{
	xsize = w;
	ysize = h;
	thescreen = SDL_SetVideoMode(xsize, ysize, 32, SDL_RESIZABLE);
	if ( thescreen == NULL )
	{
		fprintf(stderr, "Couldn't set display mode: %s\n",
							SDL_GetError());
		exit(5);
	}

}

int main(int argc,char **argv)
{
int code;
int mousex,mousey;
int done=0;
SDL_Event event;
int flags=0;
void *soundinfo;

	printf("SDL timeless version 1.00\n");

	if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 )
	{
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		exit(1);
	}

	resize(XSIZE, YSIZE);
	general_init();
	soundinfo=sound_init();
	SDL_AddTimer(40, pulse, 0);

	while(!done)
	{
		while(!interrupt)
			SDL_Delay(10);
		interrupt = 0;
		iterate(flags);
		sound_pulse(soundinfo);
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_MOUSEMOTION:
				mousex=event.motion.x;
				mousey=event.motion.y;
				break;
			case SDL_MOUSEBUTTONDOWN:
				break;
			case SDL_KEYDOWN:
				code=event.key.keysym.sym;
				if(code==SDLK_ESCAPE) done=1;
				if(code==SDLK_SPACE) flags ^= OPTION_PAUSED;
				if(code=='1') flags ^= OPTION_UNWARP_BG;
				if(code=='f') flags |= OPTION_FAST;
				break;
			case SDL_KEYUP:
				code=event.key.keysym.sym;
				if(code=='f') flags &= ~OPTION_FAST;
				break;
			case SDL_VIDEORESIZE:
				resize(event.resize.w, event.resize.h);
				break;

			}
		}
	}
	SDL_Quit();
	return 0;
}
