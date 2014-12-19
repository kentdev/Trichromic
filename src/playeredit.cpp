#include "empire.h"

/*extern void description(int c, int x, int y, bool centeredtext = true);
extern string file_select_ol(string message, string extensions, int width, int height);
extern string describe_strength(int num);
extern string describe_weakness(int num);
extern string describe_power(int num);

void filecopy(string from, string to);
void draw_picture(ol::Bitmap *bmp, int x, int y);*/
void text_box(int x, int y, int w, int h, char *ret, int leng, int spaces, string title);
void text_box(int x, int y, int w, int h, char *ret, int leng, int spaces);
/*void remove_character(int n);
void new_player_character(const char *name, const char *desc, int str, int weak, int pow[NUM_POWERS], const char *powname, ol::Bitmap *bmp);
void update_playerdat();
int change_avatar(int n);
int change_music(int n);
int edit_strength(int num);
int edit_weakness(int num);
int edit_power(int num);
string describe_power(int num);
//filecopy function from Oscar Giner's code on allegro.cc (search for "copying a file" and it should show up)
//text_box function courtesy of Derezo at allegro.cc, but modified to work with OpenLayer and look better

int playeredit()
{
  static bool first = true;
  static _button goback, next, prev, newplayer;
  static _button pic[2], music[2], name[2], desc[2], strength[2], weakness[2], power[2], powname[2], erase[2], playcurrent[2];
  static int page;
  static int selecting;
  static int current;
	static short changing_avatar = -1;
	static short changing_music = -1;
  static bool anyone;
  bool nocheck = false;
	char former[70];
  char temp[70];
  char songpath[15];
  int i, tempnum[NUM_POWERS];
  BITMAP *tmpbmp;
  
  if (first)
  {
    goback.init(5, 458, 80, 17, "Done");
    next.init(593, 220, 46, 20, "Next");
    prev.init(0, 220, 46, 20, "Last");
    newplayer.init(310, 457, 80, 20, "New Player");
    pic[0].init(57, 135, 86, 18, "Change");
    pic[1].init(57, 375, 86, 18, "Change");
    erase[0].init(0, 47, 53, 17, "Remove");
    erase[1].init(0, 287, 53, 17, "Remove");
    name[0].init(560, 45, 80, 13, "Name");
    desc[0].init(560, 59, 80, 13, "Description");
    strength[0].init(560, 73, 80, 13, "Strength");
    weakness[0].init(560, 87, 80, 13, "Weakness");
    power[0].init(560, 115, 80, 13, "Power");
    powname[0].init(560, 101, 80, 13, "Power Name");
    music[0].init(57, 170, 86, 20, "Change Music");
    playcurrent[0].init(155, 170, 130, 20, "Play Current Music");
    name[1].init(560, 285, 80, 13, "Name");
    desc[1].init(560, 299, 80, 13, "Description");
    strength[1].init(560, 313, 80, 13, "Strength");
    weakness[1].init(560, 327, 80, 13, "Weakness");
    power[1].init(560, 355, 80, 13, "Power");
    powname[1].init(560, 341, 80, 13, "Power Name");
    music[1].init(57, 410, 86, 20, "Change Music");
    playcurrent[1].init(155, 410, 130, 20, "Play Current Music");
    anyone = false;
    selecting = -1;
    current = 0;
    page = 0;
    i = 0;
    if (first_mission_character > 0)
    {
      anyone = true;
    }
    first = false;
  }
  
  if (selecting != -1)
  {
    switch(selecting)  //submenus to change strength, weakness, power types
    {
      case 0:
        selecting = edit_strength(current);
        break;
      case 1:
        selecting = edit_weakness(current);
        break;
      case 2:
        selecting = edit_power(current);
        break;
    }
		if (selecting == -1)
		{  //if the edit_* menu has just been closed
			update_playerdat();
		}
    return PLAYEREDIT;
  }
  
	if ((changing_avatar != -1) || (changing_music != -1))
	{
		nocheck = true;
	}
	
	if (!anyone)
  {  //if there are no player characters, create a default one
	 	avatars.push_back(new ol::Bitmap());
  	tmpbmp = create_bitmap(80, 80);
  	clear_to_color(tmpbmp, LGREY);
  	avatars[avatars.size() - 1]->Load(tmpbmp, ol::CONVERT_MAGIC_PINK);
		i = 0;
		while (i < NUM_POWERS)
		{
			tempnum[i] = 0;
			i++;
		}
 		new_player_character("Default", "Default player character", 5, 5, tempnum, "Nothing!", avatars[avatars.size() - 1]);
 		page = (first_mission_character / 2) - 1 + first_mission_character % 2;
    anyone = true;
  }
	else
  {
    sprintf(temp, "Page %-1d", page + 1);
    buffer_textout_ex(normal, temp, 5, 5, LGREY);

    draw_picture(character[page * 2]->picture, 60, 50);
    description(page * 2, 350, 50);
    if (page * 2 + 1 < first_mission_character)
    {
      draw_picture(character[page * 2 + 1]->picture, 60, 290);
      description(page * 2 + 1, 350, 290);
    }

    i = 0;
    while (i < 2)
    {
      if (page * 2 + i < first_mission_character)
      {
        if (nocheck)
				{
					pic[i].draw();
					erase[i].draw();
					name[i].draw();
					desc[i].draw();
					strength[i].draw();
					weakness[i].draw();
					power[i].draw();
					powname[i].draw();
					music[i].draw();
					playcurrent[i].draw();
				}
				else
				{
					pic[i].check();
        	erase[i].check();
        	name[i].check();
        	desc[i].check();
        	strength[i].check();
        	weakness[i].check();
        	power[i].check();
        	powname[i].check();
        	music[i].check();
        	playcurrent[i].check();
        	if (pic[i].clicked())
        	{
	          changing_avatar = page * 2 + i;
        	}
        	if (music[i].clicked())
        	{
						changing_music = page * 2 + i;
        	}
        	if (erase[i].clicked())
        	{
	          remove_character(page * 2 + i);
          	if (page * 2 >= first_mission_character)
          	{
	            page--;
            	if (first_mission_character == 0)
            	{
	              anyone = false;
            	}
          	}
        	}
        	if (playcurrent[i].clicked())  //play the player's music file
        	{
	          sprintf(songpath, "data/music/player%-1d", page * 2 + i + 1);
          	play_song(songpath);
        	}
        	if (name[i].clicked())  //change name
        	{
						strcpy(temp, character[page * 2 + i]->getname());
						strcpy(former, temp);
          	text_box(220, 200, 200, 40, temp, 20, 1, "Player Name:");
          	if (strcmp(temp, "_CANCEL_") == 0)
          	{
							strcpy(temp, former);
          	}
          	character[page * 2 + i]->setname(temp);
	        }
        	if (desc[i].clicked())  //change description
        	{
						strcpy(temp, character[page * 2 + i]->getdescription());
						strcpy(former, temp);
          	text_box(90, 200, 460, 40, temp, 60, 1, "Player Description:");
          	if (strcmp(temp, "_CANCEL_") == 0)
          	{
							strcpy(temp, former);
          	}
          	character[page * 2 + i]->setdescription(temp);
        	}
        	if (strength[i].clicked())  //change strength
        	{
	          selecting = 0;
          	current = page * 2 + i;
        	}
        	if (weakness[i].clicked())  //change weakness
        	{
	          selecting = 1;
          	current = page * 2 + i;
        	}
        	if (power[i].clicked())  //change power
        	{
	          selecting = 2;
          	current = page * 2 + i;
        	}
        	if (powname[i].clicked())  //change power name
        	{
						strcpy(temp, character[page * 2 + i]->getpowername());
						strcpy(former, temp);
          	text_box(220, 200, 200, 40, temp, 25, 1, "Power Name:");
          	if (strcmp(temp, "_CANCEL_") == 0)
          	{
							strcpy(temp, former);
          	}
          	character[page * 2 + i]->setpowername(temp);
        	}
      	}
			}
      i++;
    }
    
    if (nocheck)
		{
			next.draw();
			prev.draw();
		}
		else
		{
			next.check();
    	prev.check();
    	if (next.clicked())
    	{
	      page++;
      	if (page * 2 >= first_mission_character)
      	{
	        page = 0;
      	}
    	}
    	if (prev.clicked())
    	{
	      page--;
      	if (page < 0)
      	{
	        page = (first_mission_character / 2) - 1 + first_mission_character % 2;
      	}
    	}
		}
	}
	
	if (nocheck)
	{
		goback.draw();
		newplayer.draw();
	}
	else
	{
		goback.check();
		newplayer.check();
		if (goback.clicked())
 		{
	    first = true;
			play_song(title_music);
   		return MENU;
 		}
		if (newplayer.clicked())
  	{
    	avatars.push_back(new ol::Bitmap());
   		tmpbmp = create_bitmap(80, 80);
   		clear_to_color(tmpbmp, LGREY);
   		avatars[avatars.size() - 1]->Load(tmpbmp, ol::CONVERT_MAGIC_PINK);
			i = 0;
			while (i < NUM_POWERS)
			{
				tempnum[i] = 0;
				i++;
			}
   		new_player_character("New Player", "No Description", 5, 5, tempnum, "No Power Name", avatars[avatars.size() - 1]);
   		page = (first_mission_character / 2) - 1 + first_mission_character % 2;
   		anyone = true;
		}
	}
	
	if (changing_avatar != -1)
	{
		changing_avatar = change_avatar(changing_avatar);
	}
	else if (changing_music != -1)
	{
		changing_music = change_music(changing_music);
	}
	
	if (selecting != -1)
  {  //if one of the submenus is going to be entered next loop
    //draw_all(transition);  //grab the screen for a transition effect
    //buffer_draw_spite(transition, 0, 0);
  }

  draw_mouse();
  
  return PLAYEREDIT;
}

void draw_picture(ol::Bitmap *bmp, int x, int y)
{
  buffer_rect(x - 1, y - 1, x + 80, y + 80, DGREY);
  buffer_rect(x - 2, y - 2, x + 81, y + 81, GREY);
  buffer_rect(x - 3, y - 3, x + 82, y + 82, DGREY);
  if (bmp != NULL)
  {
    buffer_draw_sprite(bmp, x, y);
  }
  else
  {
    buffer_rectfill(x, y, x + 80, y + 80, GREY);
  }
}

void remove_character(int n)
{
	char a[30], b[30];
	character.erase(character.begin() + n, character.begin() + n + 1);
	
	empdebug("  Character removed, adjusting player bitmap filenames");
	while (n < first_mission_character)
	{
		sprintf(a, "data/player%d.bmp", n + 2);
		sprintf(b, "data/player%d.bmp", n + 1);
		rename(a, b);
		n++;
	}
	sprintf(a, "data/player%d.bmp", n + 1);
	delete_file(a);
	empdebug("  ...done");
	
	first_mission_character--;
	
	update_playerdat();
}*/

void text_box(int x, int y, int w, int h, char *ret, int leng, int spaces)
{
  text_box(x, y, w, h, ret, leng, spaces, "");
}

void text_box(int x, int y, int w, int h, char *ret, int leng, int spaces, string title)
{
  bool exit = false;
	char orig[leng];
  int i = 0;
  int k;
  int textx = x;
  int texty = y;
  sprintf(orig, ret);
  k = 0;
  while (orig[k] != '\0')
  {
    i++;
    k++;
  }
  int q = 0;
  int y_offset = 0;
  if (title != "")
  {
    y_offset = 15;
  }
	
	if (global.current_resolution == 1)
	{
		y_offset = int(y_offset * 1.28);
		textx = int(textx * 1.28);
		texty = int(texty * 1.28);
	}
	else if (global.current_resolution == 2)
	{
		y_offset = int(y_offset * 1.6);
		textx = int(textx * 1.6);
		texty = int(texty * 1.6);
	}

  
	ol::Rect(x, y, w, h, 2.0, 3.0).Draw(ol::Rgba(BLACK, 255));
	ol::Rect(x, y, w, h, 2.0, 3.0).DrawOutline(ol::Rgba(DGREY, 255));
  normal.SetColor(ol::Rgba(WHITE, 255));
  
	if (title != "")
  {
    normal.Print(title, textx + 3, texty + 3 + normal.Height(title));
  }

  normal.Print(orig, textx + 3, texty + y_offset + 4 + normal.Height(orig));
  ol::Canvas::Refresh();
  clear_keybuf();
	
  while (!exit)
  {
    k = readkey();
    if(((k >> 8) == KEY_ENTER)) q = 1;
    else if(((k >> 8) == KEY_BACKSPACE) && i > 0) { i--; ret[i] = '\0'; }
    else if(((k >> 8) == KEY_BACKSPACE)) { ret[i] = '\0'; }
    else if(((k >> 8) == KEY_ESC))
    {
      q = 1;
      sprintf(ret, "_CANCEL_");
    }
    else if (((k >> 8) == KEY_SPACE) && (spaces == 0))
    {  //do nothing
    }
    else
    {
      if (i < leng)
      {
        ret[i] = k & 0xFF;
        ret[i+1] = '\0';
        i++;
      }
    }
    if (q == 1)
    {
      if (ret[0] != '\0')
      {
        exit = true;
      }
      q = 0;
    }
		
		ol::Rect(x, y, w, h, 2.0, 3.0).Draw(ol::Rgba(BLACK, 255));
    ol::Rect(x, y, w, h, 2.0, 3.0).DrawOutline(ol::Rgba(DGREY, 255));

    if (title != "")
    {
      normal.Print(title, textx + 3, texty + 3 + normal.Height(title));
    }
		
    normal.Print(ret, textx + 3, texty + y_offset + 4 + normal.Height(ret));
    
		if (strcmp(ret, "_CANCEL_") != 0)
    {  //prevent the box from displaying "_CANCEL_" in its last frame
      ol::Canvas::Refresh();
    }
		
    rest(1);
  }
  counter = 0;
}
/*
int change_avatar(int n)
{
  BITMAP *temp, *temp2;
	char t[30];
	int i;
	string file;
	static _fileselect f;
	static bool first = true;
	
	if (first)
	{
		f.init("Select a new avatar (jpg, jpeg, png, bmp, pcx, lbm, tga)", "jpg;jpeg;png;bmp;lbm;pcx;tga");
		first = false;
	}
	
	file = f.check();
	
	if (file == "/CANCEL/")
	{
		f.clear();
		first = true;
		return -1;
	}
	
	if (file != "/NOTHING YET/")
	{
    temp = load_bitmap(file.c_str(), NULL);
    if (!temp)
    {
			empdebug("Error loading bitmap");
			empdebug(file);
			f.clear();
			first = true;
      return -1;
    }
		
		empdebug("  Loading new avatar bitmap...");
		temp2 = create_bitmap(80, 80);
    clear_to_color(temp2, TRANS);
    stretch_sprite(temp2, temp, 0, 0, 80, 80);
    avatars.push_back(new ol::Bitmap());
    i = avatars.size() - 1;
    avatars[i]->Load(temp2, ol::CONVERT_MAGIC_PINK);
		empdebug("  ...done");
		
    character[n]->picture = avatars[i];
		
		empdebug("  Saving new avatar over old one...");
		sprintf(t, "data/player%-1d.bmp", n + 1);
//		Transforms::SetPosition(0, -1);
		avatars[i]->Save(t);
//		Transforms::ResetPlacement();
		empdebug("  ...done");
		
    destroy_bitmap(temp);
		
		f.clear();
		first = true;
		return -1;
  }
	return n;
}

int change_music(int n)
{
	static bool first = true;
	static _fileselect f;
  char filedest[100] = "";
	string path;
	
  sprintf(filedest, "data/music/player%-1d", n + 1);
	
	if (first)
	{
		f.init("Select a new music file (mp3, wav, mid, mod, it, xm, s3m)", "mp3;wav;mid;mod;it;xm;s3m");
		first = false;
	}
	
	path = f.check();
	
	if (path == "/CANCEL/")
	{
		first = true;
		return -1;
	}
	if (path != "/NOTHING YET/")
	{
		filecopy(path, filedest);
		first = true;
		return -1;
	}

	return n;
}

void filecopy(string from, string to)
{
  const int BUFFER_SIZE = 1024;
  FILE *fp, *fdest;
  char buffer[ BUFFER_SIZE ];
  unsigned int size;

  if ((fp = fopen(from.c_str(), "rb")) == NULL)
  {
    return;
  }

  if ((fdest = fopen(to.c_str(), "wb")) == NULL)
  {
    fclose(fp);
    return;
  }

  while((size = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
  {
    if (fwrite(buffer, 1, size, fdest) != size)
    {
      fclose(fdest);
      fclose(fp);
      return;
    }
  }

  fclose(fdest);
  fclose(fp);
}

int edit_strength(int num)
{
  static bool first = true;
  static bool transitioning;
  static int y, t;
  static _button str[6];
  int i;

  if (first)
  {
    //set_mouse_range(220, 120, 420, 360);
    position_mouse(320, 130);
    y = 0;
    t = 0;
    transitioning = true;
    first = false;
    i = 0;
    while (i < 6)
    {
      str[i].init(240, 170 + (i * 30), 160, 20, describe_strength(i));
      i++;
    }
  }

  //buffer_draw_spite(transition, 0, 0);
  buffer_rect(220, 240 - y, 420, 240 + y, DGREY);
  buffer_rect(221, 241 - y, 419, 239 + y, GREY);
  buffer_rect(222, 242 - y, 418, 238 + y, DGREY);
  buffer_rectfill(223, 243 - y, 417, 237 + y, BLACK);
  if (y == 120)
  {
    buffer_textout_centre_ex(normal, "Select this player's strength:", 320, 140, WHITE);
    buffer_rect(239, 169 + (character[num]->strength * 30), 401, 191 + (character[num]->strength * 30), DGREEN);
    i = 0;
    while (i < 6)
    {
      if ((i != character[num]->weakness) || (character[num]->weakness == 5))
      {  //don't display the character's weakness to choose as a strength (unless it's none)
        str[i].check();
        if (str[i].clicked())
        {
          character[num]->strength = i;
          if (i == 5)
          {  //if strength is none, weakness must be none
            character[num]->weakness = 5;
          }
          else if (character[num]->weakness == 5)
          {  //if the weakness is none but the strength isn't
            if (character[num]->strength != 0)
            {
              character[num]->weakness = 0;  //change weakness from none
            }
            else  //but make sure it isn't the same as the strength
            {
              character[num]->weakness = 1;
            }
          }
          first = true;
          //set_mouse_range(0, 0, 639, 479);
          return -1;
        }
      }
      i++;
    }
  }

  if (transitioning)
  {
    y += 8;
    if (y >= 120)
    {
      y = 120;
      t += 10;
      buffer_rectfill(223, 123, 417, 357, BLACK, float(255 - t) / 255.0);
      if (t >= 250)
      {
        transitioning = false;
      }
    }
  }
  draw_mouse();
  return 0;
}

int edit_weakness(int num)
{
  static bool first = true;
  static bool transitioning;
  static int y, t;
  static _button weak[6];
  int i;

  if (first)
  {
    //set_mouse_range(220, 120, 420, 360);
    position_mouse(320, 130);
    y = 0;
    t = 0;
    transitioning = true;
    first = false;
    i = 0;
    while (i < 6)
    {
      weak[i].init(240, 170 + (i * 30), 160, 20, describe_weakness(i));
      i++;
    }
  }

  //buffer_draw_spite(transition, 0, 0);
  buffer_rect(220, 240 - y, 420, 240 + y, DGREY);
  buffer_rect(221, 241 - y, 419, 239 + y, GREY);
  buffer_rect(222, 242 - y, 418, 238 + y, DGREY);
  buffer_rectfill(223, 243 - y, 417, 237 + y, BLACK);
  if (y == 120)
  {
    buffer_textout_centre_ex(normal, "Select this player's weakness:", 320, 140, WHITE);
    buffer_rect(239, 169 + (character[num]->weakness * 30), 401, 191 + (character[num]->weakness * 30), DGREEN);
    i = 0;
    while (i < 6)
    {
      if ((i != character[num]->strength) || (character[num]->strength == 5))
      {  //don't display the character's strength to choose as a weakness (unless it's none)
        weak[i].check();
        if (weak[i].clicked())
        {
          character[num]->weakness = i;
          if (i == 5)
          {  //if weakness is none, strength must be none
            character[num]->strength = 5;
          }
          else if (character[num]->strength == 5)
          {  //if the strength is none but the weakness isn't
            if (character[num]->weakness != 0)
            {
              character[num]->strength = 0;  //change strength from none
            }
            else  //but make sure it isn't the same as the weakness
            {
              character[num]->strength = 1;
            }
          }
          first = true;
          //set_mouse_range(0, 0, 639, 479);
          return -1;
        }
      }
      i++;
    }
  }

  if (transitioning)
  {
    y += 8;
    if (y >= 120)
    {
      y = 120;
      t += 10;
      buffer_rectfill(223, 123, 417, 357, BLACK, float(255 - t) / 255.0);
      if (t >= 250)
      {
        transitioning = false;
      }
    }
  }
  draw_mouse();
  return 1;
}

int edit_power(int num)
{
	const int max_power[NUM_POWERS] = {10, 10, 10, 5, 5, 4};
  static bool first = true;
  static _button up[NUM_POWERS], down[NUM_POWERS], finish;
	char s[40];
  int i, j;

  if (first)
  {
    position_mouse(320, 130);
    first = false;
    i = 0;
    while (i < NUM_POWERS)
    {
			down[i].init(140, 168 + (i * 45), 20, 20, "-");
			up[i].init(165, 168 + (i * 45), 20, 20, "+");
			finish.init(420, 400, 100, 20, "Done");
      i++;
    }
  }

  //buffer_draw_spite(transition, 0, 0);
	buffer_menuback(120, 120, 540, 440, DGREY, BLACK, GREY);
  buffer_textout_centre_ex(normal, "Select this CO's powers:", 320, 130, WHITE);
  
	i = 0;
  while (i < NUM_POWERS)
  {
		buffer_textout_ex(normal, describe_power(i), 130, 150 + (i * 45), WHITE);
    up[i].check();
		down[i].check();
    if (up[i].clicked())
    {
			if (character[num]->power[i] < max_power[i])
			{
    	  character[num]->power[i]++;
			}
    }
		if (down[i].clicked())
		{
			if (character[num]->power[i] > 0)
			{
				character[num]->power[i]--;
			}
		}
		buffer_menuback(190, 165 + (i * 45), 196 + (max_power[i] * 25), 191 + (i * 45), DGREY, BLACK, GREY);
		j = 0;
		while (j < character[num]->power[i])
		{
			glb.draw(21 + i, 195 + (j * 25), 168 + (i * 45));
			j++;
		}
    i++;
  }
	sprintf(s, "Points needed to use power: %d", pow_needed(character[num]->power));
	buffer_textout_ex(normal, s, 130, 420, WHITE);
	
	finish.check();
	if (finish.clicked())
	{
		first = true;
		return -1;
	}

  draw_mouse();
  return 2;
}

void new_player_character(const char *name, const char *desc, int str, int weak, int pow[NUM_POWERS], const char *powname, ol::Bitmap *bmp)
{
	char s[30];
	int i, j;
	
  character.push_back(new _character);
	
	sprintf(s, "character size now %d", character.size());
	empdebug(s);
	
  i = character.size() - 1;
  while (i > first_mission_character)
  {
    character[i]->setname(character[i - 1]->getname());
		character[i]->setdescription(character[i - 1]->getdescription());
		character[i]->setpowername(character[i - 1]->getpowername());
		character[i]->strength = character[i - 1]->strength;
		character[i]->weakness = character[i - 1]->weakness;
		j = 0;
		while (j < NUM_POWERS)
		{
			character[i]->power[j] = character[i - 1]->power[j];
			j++;
		}
		character[i]->picture = character[i - 1]->picture;
    i--;
  }
  first_mission_character++;
  character[first_mission_character - 1]->create(name, desc, str, weak, pow, powname, bmp);
	
	empdebug("  New character created, saving new bitmap");
	sprintf(s, "data/player%-1d.bmp", first_mission_character);
	bmp->Save(s);
	empdebug("  ..done");
	
	update_playerdat();
}

void update_playerdat()
{
	int i = 0, j;
	
	empdebug("  updating player data");
	ofstream out("data/players.dat");
	out << first_mission_character << "\n";
  while (i < first_mission_character)
  {
    out << character[i]->getname() << "\n";
    out << character[i]->getdescription() << "\n";
		out << character[i]->getpowername() << "\n";
    out << character[i]->strength << "\n";
    out << character[i]->weakness << "\n";
		j = 0;
		while (j < NUM_POWERS)
		{
    	out << character[i]->power[j] << "\n";
			j++;
		}
    i++;
  }
  out.close();

  ofstream misout("data/missionplayers.dat");
	misout << character.size() - first_mission_character << "\n";
  while ((unsigned int)i < character.size())
  {
    misout << character[i]->getname() << "\n";
    misout << character[i]->getdescription() << "\n";
		misout << character[i]->getpowername() << "\n";
    misout << character[i]->strength << "\n";
    misout << character[i]->weakness << "\n";
    j = 0;
		while (j < NUM_POWERS)
		{
    	out << character[i]->power[j] << "\n";
			j++;
		}
    i++;
  }
  misout.close();
	empdebug("  ...done");
}

string describe_power(int num)
{
  switch(num)
  {
    case PWR_HEAL:
      return "Your units gain 1 health";
      break;
    case PWR_DIRECT:
      return "Direct attacks increase in power";
      break;
    case PWR_RANGED:
      return "Ranged attacks increase in power";
      break;
    case PWR_DAMAGE:
      return "Deals 1 damage to each enemy unit";
      break;
    case PWR_RANDKILL:
      return "Each enemy unit has a 10% chance of being destroyed";
      break;
    case PWR_DOUBLESHOT:
      return "Ranged units have a 25% chance of firing twice";
      break;
  }
  return "-ERROR-";
}
*/
