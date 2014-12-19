#include "empire.h"

vector<string> maplist;

void remove_extension(string &s);
void refresh_maplist();
void arrange_mapnames();
int add_filename(const char *filename, int attrib, void *param);

void refresh_maplist()
{
  maplist.clear();
  for_each_file_ex("maps/*.map", 0, 0, add_filename, &maplist);
	arrange_mapnames();
}

int add_filename(const char *filename, int attrib, void *param)
{
  string s = filename;
  s.erase(0, 5);
  remove_extension(s);
  ((vector<string> *)param)->push_back(s);
  return 0;
}

void remove_extension(string &s)
{  //removes the ".map" from the end of the map name
  char p[30];
  int a, b;
  sprintf(p, s.c_str());
  a = strlen(p) - 1;
  b = 0;
  while (b < 4)
  {
    p[a] = '\0';
    a--;
    b++;
  }
  s = p;
}

void arrange_mapnames()
{
  vector<_maparrange> arrange;
  _maparrange atemp;
  string temp;
  unsigned int i, j, min;
  int y, z;
  char c[60];
  
	if (maplist.size() <= 1) return;  //no arranging needed if it's just 0 or 1 maps
	
  i = 0;
  j = maplist.size();
  while (i < j)  //read all the map names in
  {
    arrange.push_back(atemp);
    arrange[i].name = maplist[i];
    sprintf(c, "maps/");
    strcat(c, arrange[i].name.c_str());
    strcat(c, ".map");
    ifstream in(c);
    if (in.bad())
    {
      arrange[i].bad = true;
    }
    else
    {
      in >> arrange[i].l >> arrange[i].h >> arrange[i].players;
      arrange[i].bad = false;
    }
    in.close();
    i++;
  }
  
  i = 0;
  while (i < arrange.size() - 1)
  {  //arrange filenames according to maporder
    min = i;  //not as fast as a mergesort, but it's not like it's going
    j = i;    //to be sorting thousands of map files or anything like that
    while (j < arrange.size())
    {
      if (global.maporder == 0)
      {  //arrange by name
        if (strcasecmp(arrange[j].name.c_str(), arrange[min].name.c_str()) < 0)
        {
          min = j;
        }
      }
      if (global.maporder == 1)
      {  //arrange by number of players
        if ((!arrange[j].bad) && (!arrange[min].bad))
        {
          if (arrange[j].players < arrange[min].players)
          {
            min = j;
          }
          else if (arrange[j].players == arrange[min].players)
          {  //arrange by name if they have the same number of players
            if (strcasecmp(arrange[j].name.c_str(), arrange[min].name.c_str()) < 0)
            {
              min = j;
            }
          }
        }
      }
      if (global.maporder == 2)
      {
        if ((!arrange[j].bad) && (!arrange[min].bad))
        {
          y = arrange[j].l * arrange[j].h;
          z = arrange[min].l * arrange[min].h;
          if (y < z)
          {
            min = j;
          }
          else if (y == z)
          {  //arrange by name if they have the same size
            if (strcasecmp(arrange[j].name.c_str(), arrange[min].name.c_str()) < 0)
            {
              min = j;
            }
          }
        }
      }
      j++;
    }
    if (min != i)
    {
      atemp = arrange[i];
      arrange[i] = arrange[min];
      arrange[min] = atemp;
    }
    i++;
  }
  
  i = 0;
  while (i < arrange.size())
  {
  	sprintf(c, "(%dP) %s", arrange[i].players, arrange[i].name.c_str());
    maplist[i] = c;//arrange[i].name;
    i++;
  }
}
