#include "empire.h"

FMOD_SOUND *musicfile;
vector<FMOD_SOUND *> soundeffects;
FMOD_SYSTEM *fmodmain;
FMOD_CHANNELGROUP *sound_effects;
FMOD_CHANNEL *music_channel;

void shutdown_sound()
{
	if (global.sound)
	{
		empdebug("Unloading sound...");
		set_sfx_volume(0);
		set_music_volume(0);
		//FMOD_System_Release(fmodmain);  //FREEZES EVERYTHING ON ALL UBUNTU KERNELS I'VE TRIED
		empdebug("Done");
	}
}

bool init_sound()
{
  if (FMOD_System_Create(&fmodmain) != FMOD_OK)
	{
	  return false;
	}
	if (FMOD_System_Init(fmodmain, 32, FMOD_INIT_NORMAL, 0) != FMOD_OK)
	{
		return false;
	}
	FMOD_System_CreateChannelGroup(fmodmain, NULL, &sound_effects);
	set_sfx_volume(global.volume);
	set_music_volume(global.music_volume);
	return true;
}

void load_sounds()
{
	const int SOUNDNUM = 22;
	FMOD_SOUND *t;
	int i = 0;
	string tstr;
	const string path = "data/sfx/";
	const string files[SOUNDNUM] = {"01_antiair.wav", "02_autogun.wav", "03_bombs.wav",
																	"04_chopper.wav", "05_click.wav", "06_engine.wav",
																	"07_explosion.wav", "08_jet.wav", "09_march.wav",
																	"10_mdtankfire.wav", "11_missile.wav", "12_seaboat.wav",
																	"13_tank.wav", "14_tankfire.wav", "15_tiledown.wav",
																	"16_torpedo.wav", "17_unitdown.wav", "18_explode2.wav",
																	"19_win.wav", "20_defeat.wav", "21_teleport.wav",
																	"22_shothit1.wav"};
	empdebug("  Loading sound effects...");
	while (i < SOUNDNUM)
	{
		soundeffects.push_back(t);
		tstr = path + files[i];
		FMOD_System_CreateSound(fmodmain, tstr.c_str(), FMOD_CREATESAMPLE, 0, &soundeffects[i]);
		i++;
	}
	empdebug("    ...done");
}

void play_sound(int soundtype)
{
	if (soundtype == -1) return;
	
  if (global.sound)
  {
		FMOD_CHANNEL *ch;
		FMOD_System_PlaySound(fmodmain, FMOD_CHANNEL_FREE, soundeffects[soundtype], false, &ch);
		FMOD_Channel_SetChannelGroup(ch, sound_effects);
  }
}

void stop_music()
{
	if (global.sound)
	{
		FMOD_Channel_Stop(music_channel);
	}
}

void play_song(string path)
{
	static string currentsong = "-";
	if ((global.sound) && (currentsong != path))
	{
		currentsong = path;
		stop_music();
		FMOD_Sound_Release(musicfile);
		FMOD_System_CreateSound(fmodmain, path.c_str(), FMOD_NONBLOCKING | FMOD_LOOP_NORMAL | FMOD_CREATESTREAM, 0, &musicfile);
		FMOD_System_PlaySound(fmodmain, FMOD_CHANNEL_FREE, musicfile, false, &music_channel);
		FMOD_Channel_SetVolume(music_channel, float(global.music_volume) / 100.0);
	}
}

void set_sfx_volume(int percent)
{
	if ((percent >= 0) && (percent <= 100))
	{
	  global.volume = percent;
	}
	else global.volume = 0;
	FMOD_ChannelGroup_SetVolume(sound_effects, float(global.volume) / 100.0);
}

void set_music_volume(int percent)
{
	if ((percent >= 0) && (percent <= 100))
	{
		global.music_volume = percent;
	}
	else global.music_volume = 0;
	FMOD_Channel_SetVolume(music_channel, float(global.music_volume) / 100.0);
}

void ensure_music_is_playing()
{
	FMOD_BOOL musicplaying = false;
	FMOD_Channel_IsPlaying(music_channel, &musicplaying);
	if (!musicplaying)
	{
		FMOD_System_PlaySound(fmodmain, FMOD_CHANNEL_FREE, musicfile, false, &music_channel);
		FMOD_Channel_SetVolume(music_channel, float(global.music_volume) / 100.0);
	}
}
