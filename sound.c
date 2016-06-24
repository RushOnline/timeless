#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "timeless.h"

#ifndef NO_SOUND

#include <SDL.h>
#include <SDL_sound.h>

typedef struct
{
	Sound_Sample *sample;
	SDL_AudioSpec devformat;
	Uint8 *decoded_ptr;
	Uint32 decoded_bytes;
	int done_flag;
	int status;
	int timer;
	SDL_RWops *rw;
} sstate;


static void audio_callback(void *userdata, Uint8 *stream, int len)
{
	sstate *data = userdata;
	Sound_Sample *sample = data->sample;
	int bw = 0;

	while (bw < len)
	{
		int cpysize;

		if (data->decoded_bytes == 0) /* need more data! */
		{
			/* if there wasn't previously an error or EOF, read more. */
			if ( ((sample->flags & SOUND_SAMPLEFLAG_ERROR) == 0) &&
				 ((sample->flags & SOUND_SAMPLEFLAG_EOF) == 0) )
			{
				data->decoded_bytes = Sound_Decode(sample);
				data->decoded_ptr = sample->buffer;
			} /* if */

			if (data->decoded_bytes == 0)
			{
				/* ...there isn't any more data to read! */
				memset(stream + bw, '\0', len - bw);  /* write silence. */
				data->done_flag = 1;
				return;  /* we're done playback, one way or another. */
			} /* if */
		}

		/* we have data decoded and ready to write to the device... */
		cpysize = len - bw;  /* len - bw == amount device still wants. */
		if (cpysize > data->decoded_bytes)
			cpysize = data->decoded_bytes;  /* clamp to what we have left. */

		/* if it's 0, next iteration will decode more or decide we're done. */
		if (cpysize > 0)
		{
			/* write this iteration's data to the device. */
			memcpy(stream + bw, (Uint8 *) data->decoded_ptr, cpysize);

			/* update state for next iteration or callback */
			bw += cpysize;
			data->decoded_ptr += cpysize;
			data->decoded_bytes -= cpysize;
		}
	}
}
#define STATUS_RESTART 0
#define STATUS_PLAYING 1
#define STATUS_ENDING  2



static void start(sstate *data)
{

//	data->sample = Sound_NewSampleFromFile("rain.mtm", NULL, 65536);

	data->rw = SDL_RWFromConstMem(song, song_size);
	data->sample = Sound_NewSample(data->rw, ".mtm", NULL, 65536);
	if (data->sample == NULL)
	{
		fprintf(stderr, "Failed to init sound: %s\n", Sound_GetError());
		return;
	}

	data->devformat.freq = data->sample->actual.rate;
	data->devformat.format = data->sample->actual.format;
	data->devformat.channels = data->sample->actual.channels;
	data->devformat.samples = 4096;  /* I just picked a largish number here. */
	data->devformat.callback = audio_callback;
	data->devformat.userdata = data;
	if (SDL_OpenAudio(&data->devformat, NULL) < 0)
	{
		fprintf(stderr, "Couldn't open audio device: %s.\n", SDL_GetError());
		Sound_FreeSample(data->sample);
		return;
	}

	SDL_PauseAudio(0);

	data->done_flag = 0;
}

static void end(sstate *data)
{
	SDL_PauseAudio(1);

//	SDL_Delay(2 * 1000 * data->devformat.samples / data->devformat.freq);

	if (data->sample->flags & SOUND_SAMPLEFLAG_ERROR)
		fprintf(stderr, "Error decoding file: %s\n", Sound_GetError());

	Sound_FreeSample(data->sample);
	SDL_CloseAudio();
}


void sound_pulse(void *par)
{
sstate *data = par;
	if(data->status == STATUS_RESTART)
	{
		start(data);
		data->status = STATUS_PLAYING;
		return;
	}
	if(data->status == STATUS_PLAYING)
	{
		if(!data->done_flag)
			return;
		SDL_PauseAudio(1);
		data->status = STATUS_ENDING;
		data->timer = 0;
	}
	if(data->status == STATUS_ENDING)
	{
		++data->timer;
		if(data->timer == 20)
		{
			end(data);
			data->status = STATUS_RESTART;
		}
	}
}

void *sound_init(void)
{
static sstate data;

	if (!Sound_Init())
	{
		fprintf(stderr, "Sound_Init() failed: %s.\n", Sound_GetError());
		SDL_Quit();
	}

	memset(&data, 0, sizeof(data));
	return &data;
}
#else

void *sound_init(void)
{
	return 0;
}

void sound_pulse(void *p)
{
}
#endif
