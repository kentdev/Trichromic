#include "empire.h"

int move_priority(_unit* u);

void _uselessstats::reset()
{
	raised = 0;
	spent = 0;
	produced = 0;
	killed = 0;
	lost = 0;
}

int _player::create_unit(int type, int x, int y, int player)
{
  int i = 0;
  while (i < 50)
  {
    if (unit[i].exists == 0)
    {
      unit[i].create(type, x, y, player, i);
      return i;  //return the number of the newly created unit
      i = 50;
    }
    i++;
  }
  return -1;
}

void _player::reset()
{
  clear_units();
  stats.reset();
  playing = 0;
  everplaying = 0;
  cash = 0;
  team = 0;
  tookturn = 0;
	tintcolor = BLACK;
	controller = HUMAN;
}

void _player::clear_units()
{
  int i = 0;
  while (i < 50)
  {
    if (unit[i].exists == 1)
    {
      unit[i].kill();
    }
    unit[i].exists = 0;
    unit[i].ready = 0;
    i++;
  }
}

void _player::ready_units()
{
  int i = 0;
  while (i < 50)
  {
    unit[i].ready = 1;
		unit[i].canmove = true;
    i++;
  }
}

void _player::reapply_unitstats()
{
	int i = 0;
	while (i < 50)
	{
		if (unit[i].exists == 1)
		{
			unit[i].create(int(unit[i].type), int(unit[i].tilex), int(unit[i].tiley), int(unit[i].color), i);
		}
		i++;
	}
}

int _player::units_in_play()
{
  int i = 0;
  int n = 0;
  while (i < 50)
  {
    if (unit[i].exists == 1)
    {
      n++;
    }
    i++;
  }
  return n;
}

bool _player::defeated(int pnum)
{
  int i, j, z;
  if (units_in_play() > 0)
  {
    return false;
  }
  j = 0;
  while (j < worldmap.h)
  {
    i = 0;
    while (i < worldmap.l)
    {
      if (worldmap.tile[i][j].is_unit_producing())
      {  //if the building can make units
        if (worldmap.tile[i][j].owned_by(pnum))
        {  //and is owned by the player
          z = worldmap.tile[i][j].unit_here();
          if (z == -1)
          {  //and doesn't have a unit on it (the unit must be an enemy at this point)
            return false;
          }
          else if (player[z / 100].unit[z % 100].ready == 1)
          {  //if the unit could move off the tile (like if a move is cancelled)
            return false;
          }
        }
      }
      i++;
    }
    j++;
  }
  return true;  //the player has no units and can't make any units, so he's defeated
}

void _player::sort_units()
{
  mergesort_units(0, 49);
}

void _player::mergesort_units(int min, int max)
{
  vector<_unit> temp;
  int left, right, size, i, pivot, a, b;
  size = max - min + 1;
  pivot = (min + max) / 2;
  temp.clear();
  if (min == max)
  {
    return;
  }
  mergesort_units(min, pivot);
  mergesort_units(pivot + 1, max);
  i = 0;
  while (i < size)
  {
    temp.push_back(unit[min + i]);
    i++;
  }
  left = 0;
  right = pivot - min + 1;
  i = 0;
  while (i < size)
  {
    if (right <= max - min)
    {
      if (left <= pivot - min)
      {
        a = move_priority(&temp[left]);
        b = move_priority(&temp[right]);
        if (a > b)
        {
          unit[i + min] = temp[right];
          unit[i + min].number = i + min;
          if (unit[i].exists == 1)
          {
            worldmap.tile[unit[i].tilex][unit[i].tiley].set_unit(unit[i].color, unit[i].number);
          }
          right++;
        }
        else
        {
          unit[i + min] = temp[left];
          unit[i + min].number = i + min;
          if (unit[i].exists == 1)
          {
            worldmap.tile[unit[i].tilex][unit[i].tiley].set_unit(unit[i].color, unit[i].number);
          }
          left++;
        }
      }
      else
      {
        unit[i + min] = temp[right];
        unit[i + min].number = i + min;
        if (unit[i].exists == 1)
        {
          worldmap.tile[unit[i].tilex][unit[i].tiley].set_unit(unit[i].color, unit[i].number);
        }
        right++;
      }
    }
    else
    {
      unit[i + min] = temp[left];
      unit[i + min].number = i + min;
      if (unit[i].exists == 1)
      {
        worldmap.tile[unit[i].tilex][unit[i].tiley].set_unit(unit[i].color, unit[i].number);
      }
      left++;
    }
    i++;
  }
}


//==============================================================================
//==============================================================================

_character::_character()
{
	//int i = 0;
	
  name = NULL;
  description = NULL;
  /*strength = 5;
  weakness = 5;
  while (i < NUM_POWERS)
	{
		power[i] = 0;
		i++;
	}
	powerneeded = 0;
  powername = NULL;*/
  picture = NULL;
}

_character::~_character()
{
  destroy();
}

/*void _character::create(const char *n, const char *d, int s, int w, int p[NUM_POWERS], const char *pn, ol::Bitmap *pic)
{
	int i = 0;
	destroy();
  setname(n);
	setdescription(d);
	setpowername(pn);
  strength = s;
  weakness = w;
	while (i < NUM_POWERS)
	{
  	power[i] = p[i];
		i++;
	}
	powerneeded = pow_needed(power);
  picture = pic;
}*/

void _character::destroy()
{
	/*int i = 0;
	
  strength = 5;
  weakness = 5;
  
	while (i < NUM_POWERS)
	{
		power[i] = 0;
		i++;
	}*/
	if (name != NULL)
	{
		free(name);
		name = NULL;
	}
	if (description != NULL)
	{
		free(description);
		description = NULL;
	}
	/*if (powername != NULL)
	{
		free(powername);
		powername = NULL;
	}*/
	if (picture)
	{
		picture->Destroy();
		delete picture;
	}
  picture = NULL;
}

int move_priority(_unit* u)
{
  if (unitstats[u->type].attacktype == RANGED)
  {
    return 0;
  }
  else if (unitstats[u->type].attacktype == DIRECT)
  {
    if (unitstats[u->type].cancapture)
    {
      return 2;
    }
    else
    {
      return 1;
    }
  }
  return 3;
}

char *_character::getname()
{
	return name;
}

char *_character::getdescription()
{
	return description;
}

/*char *_character::getpowername()
{
	return powername;
}*/

void _character::setname(const char *n)
{
	if (name != NULL)
	{
		free(name);
	}
	name = (char *)malloc(sizeof(char) * (strlen(n) + 1));
	strcpy(name, n);
}

void _character::setdescription(const char *d)
{
	if (description != NULL)
	{
		free(description);
	}
	description = (char *)malloc(sizeof(char) * (strlen(d) + 1));
	strcpy(description, d);
}

/*void _character::setpowername(const char *pn)
{
	if (powername != NULL)
	{
		free(powername);
	}
	powername = (char *)malloc(sizeof(char) * (strlen(pn) + 1));
	strcpy(powername, pn);
}*/

/*void load_character(ifstream *in, _character *c)
{
	string name, desc, pname, str, weak, pwr;
	int s, w, p[NUM_POWERS], i;
	
	getline(*in, name);
	getline(*in, desc);
	getline(*in, pname);
	getline(*in, str);
	s = atoi(str.c_str());
	getline(*in, weak);
	w = atoi(weak.c_str());
	
	i = 0;
	while (i < NUM_POWERS)
	{
		getline(*in, pwr);
		p[i] = atoi(pwr.c_str());
		i++;
	}
	
	c->create(name.c_str(), desc.c_str(), s, w, p, pname.c_str(), avatars[avatars.size() - 1]);
 	empdebug("    Loaded player.");
}

void load_saved_characters(const char *savefilename)
{
	int i, n;
	char *c = (char*)malloc(sizeof(char) * (strlen(savefilename) + 20));
	char *d = (char*)malloc(sizeof(char) * (strlen(savefilename) + 20));
	string z;
	
	strcpy(c, savefilename);
	strcat(c, ".characters");
	ifstream in(c);
	
	if (in.bad())
	{
		z = "Couldn't load ";
		z += c;
		empdebug(z);
		free(c);
		free(d);
		return;
	}
	
	getline(in, z);
	n = atoi(z.c_str());
	i = 0;
	while (i < n)
	{
		if (loadedcharacter[i] != NULL)
		{
			delete(loadedcharacter[i]);
		}
		
		sprintf(d, "%s.p%d.bmp", savefilename, i);
		avatars.push_back(new ol::Bitmap(80, 80));
		avatars[avatars.size() - 1]->Load(d, ol::CONVERT_MAGIC_PINK);
		
		loadedcharacter[i] = new _character();
		load_character(&in, loadedcharacter[i]);
		currentcharacter[i] = loadedcharacter[i];
		i++;
	}
}

int pow_needed(int power[NUM_POWERS])
{
	const int pow[NUM_POWERS] = {500, 250, 250, 500, 500, 200};
	int i = 0, total = 0;
	
	while (i < NUM_POWERS)
	{
		total += pow[i] * power[i];
		i++;
	}
	return total;
}

void save_currentcharacters(const char *savefilename)
{
	char *filename_base;
	char *fname;
	char bmpname[50];
	int len, i, j, p;
	
	if (savefilename == NULL) return;
	len = strlen(savefilename) + 20;
	
	filename_base = (char*)malloc(sizeof(char) * len);
	strcpy(filename_base, savefilename);
	
	fname = (char*)malloc(sizeof(char) * (strlen(filename_base) + 20));
	strcpy(fname, filename_base);
	strcat(fname, ".characters");
	
	ofstream out(fname);
	if (!out.bad())
	{
		p = 0;
		i = 0;
		while (i < 4)
		{
			if (player[i].everplaying == 1)
			{
				p++;
			}
			sprintf(bmpname, "%s.p%d.bmp", filename_base, i);
			delete_file(bmpname);  //get rid of old player bitmaps
			i++;
		}
		out << p << "\n";
		i = 0;
		while (i < p)
		{
		  out << currentcharacter[i]->getname() << "\n";
    	out << currentcharacter[i]->getdescription() << "\n";
			out << currentcharacter[i]->getpowername() << "\n";
    	out << currentcharacter[i]->strength << "\n";
    	out << currentcharacter[i]->weakness << "\n";
    	j = 0;
			while (j < NUM_POWERS)
			{
				out << currentcharacter[i]->power[j] << "\n";
				j++;
			}
			
			if (currentcharacter[i]->picture != NULL)
			{
				sprintf(bmpname, "%s.p%d.bmp", filename_base, i);
//				Transforms::SetPosition(0, -1);
				currentcharacter[i]->picture->Save(bmpname);
//				Transforms::ResetPlacement();
			}
			
			i++;
		}
	}
	out.close();
	free(fname);
	free(filename_base);
}*/
