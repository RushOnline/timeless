#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
This takes the csprs.m file from the original timeless source and
converts it into a more useful data structure. The original csprs.m was
a series of data statements that represent x86 programs to byte by
byte write pixels to the VGA video buffer. The code below interprets the
opcodes and builds up an equivalent bitmap of the sprite, then dumps
that as data statements. Each row of our output is
#of pixels, Xoffset, Yoffset, [pixel values]
Then a 00 is an endmark.

I include this just for closure and because it's so small...
I wonder about the tool Tran used to generate that code...why have it
generate byte codes and not TASM assembly code?

*/


#define WIDTH 400

int array[65536];

void set(int a, int v)
{
	array[a]=v;
}

void dump(int n, unsigned char *p)
{
int al=0;
int bit=0;
int edi=0;
int v;
int x,y,c;

	memset(array, 0xff, sizeof(array));

	for(;;)
	{

//  d9:   ee                      out    %al,(%dx)
		if(p[0]==0xee) // out al,(dx)
		{
			bit=al;
			++p;
			continue;
		}

//  d7:   d0 c0                   rol    %al
		if(p[0]==0xd0 && p[1]==0xc0) // rol al
		{
			++al;
			p+=2;
			continue;
		}

// 173:   81 c7 c8 00 00 00       add    $0xc8,%edi
		if(p[0]==0x81 && p[1]==0xc7) // add ####,edi
		{
			v=p[2] | (p[3]<<8) | (p[4]<<16) | (p[5]<<24);
			edi+=v*4;
			p+=6;
			continue;
		}

// 1ad:   81 d7 2f f8 ff ff       adc    $0xfffff82f,%edi
		if(p[0]==0x81 && p[1]==0xd7) // adc ####,edi
		{
			v=p[2] | (p[3]<<8) | (p[4]<<16) | (p[5]<<24);
			edi+=v*4;
			p+=6;
			continue;
		}

//  4d:   c6 47 02 d1             movb   $0xd1,0x2(%edi)
		if(p[0]==0xc6 && p[1]==0x47) // movb xx, yy(edi)
		{
			v=edi + p[2]*4 + bit;
			set(v, p[3]);
			p+=4;
			continue;
		}

//   7:   c6 07 d0                movb   $0xd0,(%edi)
		if(p[0]==0xc6 && p[1]==0x07) // movb xx, (edi)
		{
			v=edi + bit;
			set(v, p[2]);
			p+=3;
			continue;
		}

//  96:   c7 07 c0 d1 d2 d2       movl   $0xd2d2d1c0,(%edi)
		if(p[0]==0xc7 && p[1]==0x07) // movl ########, (edi)
		{
			v=edi+bit;
			set(v, p[2]);
			set(v+4, p[3]);
			set(v+8, p[4]);
			set(v+12, p[5]);
			p+=6;
			continue;
		}

//  85:   c7 47 64 c0 d2 d3 d2    movl   $0xd2d3d2c0,0x64(%edi)
		if(p[0]==0xc7 && p[1]==0x47) // movl ########, xx(edi)
		{
			v=edi + p[2]*4 + bit;
			set(v, p[3]);
			set(v+4, p[4]);
			set(v+8, p[5]);
			set(v+12, p[6]);
			p+=7;
			continue;
		}

//  ba:   66 c7 47 64 d0 d1       movw   $0xd1d0,0x64(%edi)
		if(p[0]==0x66 && p[1]==0xc7 && p[2]==0x47) // movw ####, ##(edi)
		{
			v=edi+p[3]*4 + bit;
			set(v, p[4]);
			set(v+4, p[5]);

			p+=6;
			continue;
		}

// 124:   66 c7 07 d1 d2          movw   $0xd2d1,(%edi)
		if(p[0]==0x66 && p[1]==0xc7 && p[2]==0x07) // movw ####, (edi)
		{
			v=edi + bit;
			set(v, p[3]);
			set(v+4, p[4]);
			p+=5;
			continue;
		}

// 34f:   c3                      ret    
		if(p[0]==0xc3)
		{
			++p;
			break;
		}
		fprintf(stderr, "Unknown code %02x %02x\n", p[0], p[1]);
		break;
	}
	printf("unsigned char cspr%02x[]=\n", n);
	printf("{\n");

	for(y=0;y<64;++y)
	{
		int *ap;
		ap=array+y*WIDTH;
		for(x=0;x<256;)
		{
			if(ap[x]<0)
			{
				++x;
				continue;
			}
			for(c=1;c<256-x;++c)
			{
				if(ap[x+c]<0) break;
			}
			printf("\t%2d,%2d,%2d,", c, x, y);
			while(c--)
				printf("0x%02x,", ap[x++]);
			printf("\n");
		}
	}
	printf("\t0\n};\n\n");
}

void append(unsigned char *put, int *in, char *take)
{
	while(take && *take)
	{
		while(*take && (*take<'0' || *take>'9')) ++take;
		put[(*in)++] = atoi(take);
		while(*take && *take != ',') ++take;
	}	
}


int main(int argc, char **argv)
{
int n, in;
char line[256];
FILE *f;
char *p;
unsigned char data[65536];
int state;
int done;


	f=fopen("csprs.m", "r");
	if(!f)
	{
		fprintf(stderr, "Couldn't open csprs.m file from TIMESRC.ZIP\n");
		exit(-1);
	}

	printf("#include \"timeless.h\"\n\n");
	n=0;
	in=0;
	state=0;
	done=0;
	while(!done)
	{
		p=fgets(line, sizeof(line), f);
		switch(state)
		{
		case 0: // waiting for ^cspr##:
			if(!p)
				done=1;
			else if(!strncmp(p, "cspr", 4))
				in=0,state=1;
			break;
		case 1: // fetching data
			if(p && strstr(p, " db "))
				append(data, &in, p);
			else
			{
				dump(n++, data);
				state=0;
			}
			break;
		}
	}
	return 0;
}
