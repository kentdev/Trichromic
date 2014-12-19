#include "empire.h"

_ai_type defaultai;
_ai_type theai;

extern string unit_name(int i);

void init_default_ai()
{
  int i = 0;
  char na[30];
  ifstream aifile("ai/default.ai");
  while (i < NUM_UNITS)
  {
    aifile >> na >> na >> defaultai.weight[i] >> na >> defaultai.limit[i];
    i++;
  }
  aifile.close();
  theai = defaultai;
}

void load_default_ai()
{
  theai = defaultai;
}

void load_ai(string mapname)
{
  //char na[30];
  string path;
  int i = 0;
  path = "maps/" + mapname + ".ai";
  ifstream aifile(path.c_str());
  if (aifile.bad())
  {
    theai = defaultai;
    ofstream aidebug("ai_error.txt", ios::app);
    aidebug << "Couldn't find " << path.c_str() << ", using default instead.\n";
    aidebug.close();
    return;
  }
  while (i < NUM_UNITS)
  {
    aifile >> theai.weight[i] >> theai.limit[i];
    i++;
  }
  aifile.close();
}

void load_ai_from_file(string path)
{
  int i = 0;
  ifstream aifile(path.c_str());
  if (aifile.bad())
  {
    theai = defaultai;
    ofstream aidebug("ai_error.txt", ios::app);
    aidebug << "Couldn't find " << path.c_str() << ", using default instead.\n";
    aidebug.close();
    return;
  }
  while (i < NUM_UNITS)
  {
    aifile >> theai.weight[i] >> theai.limit[i];
    i++;
  }
  aifile.close();
}

void save_new_aiprofile(string name)
{
  int i = 0;
  string path = "ai/" + name + ".ai";
  //ofstream out("ai/ai_types.txt", ios::app);
  ofstream file(path.c_str());
  if (!file.bad())
  {
    while (i < NUM_UNITS)
    {
      file << theai.weight[i] << "\n " << theai.limit[i] << "\n";
      i++;
    }
    //out << "\n" << name.c_str() << "\n" << name.c_str() << ".ai";
  }
  //out.close();
  file.close();
}
