#ifndef _sound_h
#define _sound_h

#define S_AAFIRE     0
#define S_AUTOGUN    1
#define S_BOMBS      2
#define S_CHOPPER    3
#define S_CLICK      4
#define S_ENGINE     5
#define S_BLAST      6
#define S_JET        7
#define S_MARCH      8
#define S_MDTANKFIRE 9
#define S_MISSILE    10
#define S_SEABOAT    11
#define S_TANKMOVE   12
#define S_TANKFIRE   13
#define S_TILEDOWN   14
#define S_TORPEDO    15
#define S_UNITDOWN   16
#define S_EXPLOSION  17
#define S_VICTORY    18
#define S_DEFEAT     19
#define S_TELEPORT   20
#define S_SHOTHIT1   21

void set_sfx_volume(int percent);
void set_music_volume(int percent);
void stop_music();
void play_song(string path);
void play_sound(int soundtype);
void shutdown_sound();
void load_sounds();
void ensure_music_is_playing();
bool init_sound();

#endif
