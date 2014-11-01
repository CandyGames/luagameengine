#include <stdlib.h>
#include <string.h>
/*
===============================================================================

WAV loading

===============================================================================
*/

struct WavParseInfo
{
	const unsigned char	*data_p;
	const unsigned char 	*iff_end;
	const unsigned char 	*last_chunk;
	const unsigned char 	*iff_data;
	int 	iff_chunk_len;
};

static short GetLittleShort(WavParseInfo &info)
{
	short val = 0;
	val = *info.data_p;
	val = val + (*(info.data_p+1)<<8);
	info.data_p += 2;
	return val;
}

static int GetLittleLong(WavParseInfo &info)
{
	int val = 0;
	val = *info.data_p;
	val = val + (*(info.data_p+1)<<8);
	val = val + (*(info.data_p+2)<<16);
	val = val + (*(info.data_p+3)<<24);
	info.data_p += 4;
	return val;
}

static void FindNextChunk(WavParseInfo &info,char *name)
{
	while (1)
	{
		info.data_p=info.last_chunk;

		if (info.data_p >= info.iff_end)
		{	// didn't find the chunk
			info.data_p = NULL;
			return;
		}
		
		info.data_p += 4;
		info.iff_chunk_len = GetLittleLong(info);
		if (info.iff_chunk_len < 0)
		{
			info.data_p = NULL;
			return;
		}
		info.data_p -= 8;
		info.last_chunk = info.data_p + 8 + ( (info.iff_chunk_len + 1) & ~1 );
		if (!strncmp((char *)info.data_p, name, 4))
			return;
	}
}

static void FindChunk(WavParseInfo &info, char *name)
{
	info.last_chunk = info.iff_data;
	FindNextChunk (info,name);
}

struct wavinfo_t
{
	short format;
	short channels;
	int rate;
	int width;
	int samples;
	int dataofs;
	unsigned char *data;
	int dataSize;
};

/*
============
GetWavinfo
============
*/
void GetWavinfo ( wavinfo_t &info, const unsigned char *wav, int wavlength)
{
	WavParseInfo parseInfo;

	memset (&info, 0, sizeof(info));

	parseInfo.iff_data = wav;
	parseInfo.iff_end = wav + wavlength;

// find "RIFF" chunk
	FindChunk(parseInfo,"RIFF");
	if (!(parseInfo.data_p && !strncmp((char *)parseInfo.data_p+8, "WAVE", 4)))
	{
		return;
//		Com_Printf("Missing RIFF/WAVE chunks\n");
//		return info;
	}

// get "fmt " chunk
	parseInfo.iff_data = parseInfo.data_p + 12;
// DumpChunks ();

	FindChunk(parseInfo,"fmt ");
	if (!parseInfo.data_p)
	{
		return;
//		Com_Printf("Missing fmt chunk\n");
//		return info;
	}
	parseInfo.data_p += 8;
	info.format = GetLittleShort(parseInfo);
	info.channels = GetLittleShort(parseInfo);
	info.rate = GetLittleLong(parseInfo);
	parseInfo.data_p += 4+2;
	info.width = GetLittleShort(parseInfo) / 8;

	if (info.format != 1)
	{
		return;
//		Com_Printf("Microsoft PCM format only\n");
//		return info;
	}


// find data chunk
	FindChunk(parseInfo,"data");
	if (!parseInfo.data_p)
	{
		return;
//		Com_Printf("Missing data chunk\n");
//		return info;
	}

	parseInfo.data_p += 4;
	info.samples = GetLittleLong(parseInfo) / info.width;
	info.dataofs = parseInfo.data_p - wav;

//	return info;
}

wavinfo_t* LoadSnd( const char *filename )
{
	static std::map<std::string,struct wavinfo_t*> cache;

	std::map<std::string,struct wavinfo_t*>::iterator f = cache.find( filename );
	if ( f != cache.end() )
	{
		return f->second;
	}

	const bgfx::Memory*snd = load(filename);
	wavinfo_t *wi = new wavinfo_t;
	GetWavinfo( *wi, snd->data, snd->size );
	wi->data = snd->data;
	wi->dataSize = snd->size;
	cache[filename] = wi;
	return wi;
}

void PlaySound( const char *filename, float volume, bool cap )
{
	wavinfo_t *snd = LoadSnd( filename );
	if ( snd->format != 1 && snd->width != 2 )
		return;

	if ( !cap || !stb_mixlow_present( snd ) )
		stb_mixlow_add_playback( (short*)&snd->data[snd->dataofs], 
			snd->samples, 1, snd->channels, 
			0, stb_mixlow_get_curtime(), (snd->samples*44100.f/snd->rate), snd->rate/44100.f,//snd->samples, 1, 
			STB_FADE_none, 0, 0,
			volume, 0.f, snd );
}

struct Music
{
	stb_vorbis *v;
	stb_vorbis_info vinfo;
	stb_mixint start_time;
	bool playing;
};

void LoadMusic( Music &music, const char *oggfilename )
{
	int error;
	music.v = stb_vorbis_open_filename((char*)oggfilename, &error, NULL );
	music.vinfo = stb_vorbis_get_info(music.v);
	music.start_time = 0;
	music.playing = false;
}

void UpdateMusic( Music &music )
{
	if ( !music.playing )
		return;
	while ( stb_mixlow_present_count( music.v ) < 8 )
	{
		short sbuffer[1024];
		int n = stb_vorbis_get_samples_short_interleaved(music.v,2,sbuffer,1024);
		if ( n == 0 )
		{
			stb_vorbis_seek_start(music.v);
			n = stb_vorbis_get_samples_short_interleaved(music.v,2,sbuffer,1024);
		}
		stb_mixlow_add_playback(sbuffer,n, 0, 2, 0.f, music.start_time, (int)(n*44100.f/music.vinfo.sample_rate), music.vinfo.sample_rate/44100.f, 0, 0, 0, 0.04f, 0.f, music.v );
		music.start_time += (int)(n*44100.f/music.vinfo.sample_rate);
	}
}

void PlayMusic( Music &music )
{
	if ( music.playing )
		return;
	music.start_time = stb_mixlow_get_curtime();
	music.playing = true;
	UpdateMusic( music );
}

