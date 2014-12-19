#ifndef _ai_types_h
#define _ai_types_h

typedef struct _ai_type
{
  string name;
  int weight[MAX_UNITS];
  int limit[MAX_UNITS];
} _ai_type;

extern _ai_type theai;

void init_default_ai();
void load_default_ai();
void load_ai(string mapname);
void load_ai_from_file(string path);
void save_new_aiprofile(string name);

#endif
