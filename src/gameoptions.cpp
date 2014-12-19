#include "empire.h"

//int ingame_options();
int do_gameoptions(bool ingame);

void _ingameoptions::reset()
{
  show_damage = true;
  funding = 4000;
  time = -999;
}

/*int ingame_options()
{
  static bool first = true;
  static _button back, ok, opt[3];
  char c[10];
  
  if (first)
  {
    gameoptions.reset();
    back.init(10, 455, 100, 20, "Back");
    ok.init(545, 450, 95, 30, "OK");
    sprintf(c, "%-1d", gameoptions.funding);
    opt[0].init(113, 165, 80, 20, c);
    opt[1].init(250, 200, 80, 20, "Yes");
    opt[2].init(387, 235, 80, 20, "None");
    first = false;
  }
  
  glb.draw(18, 0, 0);
  buffer_textout_ex(normal, "Funds per Building", 100, 150, WHITE);
  buffer_textout_ex(normal, "Show Attacking Damage", 220, 185, WHITE);
  buffer_textout_ex(normal, "Turn Time Limit", 380, 220, WHITE);

  opt[0].check();
  opt[1].check();
  opt[2].check();
  back.check();
  ok.check();
  if (opt[0].clicked())
  {
    gameoptions.funding += 1000;
    if (gameoptions.funding > 10000)
    {
      gameoptions.funding = 1000;
    }
    sprintf(c, "%-1d", gameoptions.funding);
    opt[0].init(113, 165, 80, 20, c);
  }
  if (opt[1].clicked())
  {
    if (gameoptions.show_damage)
    {
      gameoptions.show_damage = false;
      opt[1].init(250, 200, 80, 20, "No");
    }
    else
    {
      gameoptions.show_damage = true;
      opt[1].init(250, 200, 80, 20, "Yes");
    }
  }
  if (opt[2].clicked())
  {
    if (gameoptions.time == -999)
    {
      gameoptions.time = 1000;
    }
    else
    {
      gameoptions.time += 1000;
      if (gameoptions.time > 15000)
      {
        gameoptions.time = -999;
      }
    }
    if (gameoptions.time == -999)
    {
      opt[2].init(387, 235, 80, 20, "None");
    }
    else
    {
      sprintf(c, "%-1d seconds", gameoptions.time / 50);
      opt[2].init(387, 235, 80, 20, c);
    }
  }
  if (back.clicked())
  {
    first = true;
    return MENU_OPTIONS;
  }
  if (ok.clicked())
  {
    first = true;
    clear_particles();
    return LOGIC;
  }
  draw_mouse();
  return MENU_INGAME_OPTIONS;
}*/

int do_gameoptions(bool ingame)  //this menu can also be entered while in-game
{
  static int first = 1;
  static int previous_maporder;
  static _button back;
  static _button speed[2];
  static _button fxvol[2];
  static _button musvol[2];
  static _button scroll;
  static _button arrange;
  static _button screenmode;
  static _button part;
  static _button testsound;
  static _button windowres;
  static _button showbattle;
	static _textline text;
  char c[10];
  char tempchar[30];
  if (first == 1)
  {
		text.init(415, 200, 90, 13, false);
		text.set_text(global.netname);
    previous_maporder = global.maporder;
    back.init(10, 575, 100, 20, "Back");
    if (global.fullscreen == 1)
    {
      screenmode.init(295, 300, 100, 20, "Fullscreen");
    }
    else
    {
      screenmode.init(295, 300, 100, 20, "Windowed");
    }
    if (global.windowed_resolution == 0)
    {
      windowres.init(295, 325, 210, 20, "Windowed Resolution: 800x600");
    }
    else if (global.windowed_resolution == 1)
    {
      windowres.init(295, 325, 210, 20, "Windowed Resolution: 1024x768");
    }
    else
    {
      windowres.init(295, 325, 210, 20, "Windowed Resolution: 1280x960");
    }
    fxvol[0].init(340 - 70, 20, 30, 20, "<");
    fxvol[1].init(430 - 70, 20, 30, 20, ">");
    testsound.init(350 - 70, 45, 100, 18, "Test");
    musvol[0].init(340 + 70, 20, 30, 20, "<");
    musvol[1].init(430 + 70, 20, 30, 20, ">");
    switch(global.scroll_divider)
    {
      case 1:
        scroll.init(280, 80, 240, 20, "Map Scroll Speed: Very Fast");
        break;
      case 2:
        scroll.init(280, 80, 240, 20, "Map Scroll Speed: Fast");
        break;
      case 4:
        scroll.init(280, 80, 240, 20, "Map Scroll Speed: Normal");
        break;
      case 8:
        scroll.init(280, 80, 240, 20, "Map Scroll Speed: Slow");
        break;
      case 16:
        scroll.init(280, 80, 240, 20, "Map Scroll Speed: Slooooooooow");
        break;
    }
    if (global.particles == 1)
    {
      part.init(350, 250, 100, 20, "Particles: ON");
    }
    else
    {
      part.init(350, 250, 100, 20, "Particles: OFF");
    }
    switch(global.maporder)
    {
      case 0:
        sprintf(c, "Name");
        break;
      case 1:
        sprintf(c, "Number of Players");
        break;
      case 2:
        sprintf(c, "Map Size");
        break;
    }
    arrange.init(330, 130, 140, 20, c);
    speed[0].init(350, 370, 30, 20, "<");
    speed[1].init(420, 370, 30, 20, ">");
    if (global.battle_animation == 1)
    {
      showbattle.init(350, 170, 100, 20, "ON");
    }
    else
    {
      showbattle.init(350, 170, 100, 20, "OFF");
    }
    first = 0;
  }
  glb.draw(7, 0, 0);
  buffer_textout_centre_ex(normal, "Sound Volume", 330, 5, WHITE);
  sprintf(tempchar, "%-1d", global.volume);
  buffer_textout_centre_ex(normal, tempchar, 330, 25, WHITE);
  buffer_textout_centre_ex(normal, "Music Volume", 470, 5, WHITE);
  sprintf(tempchar, "%-1d", global.music_volume);
  buffer_textout_centre_ex(normal, tempchar, 470, 25, WHITE);
  buffer_textout_centre_ex(normal, "Arrange Maps by:", 400, 115, WHITE);
  buffer_textout_centre_ex(normal, "Battle Animation:", 400, 155, WHITE);
  buffer_textout_centre_ex(normal, "Stuff That Doesn't Really Affect Gameplay But Makes Things Look Nicer", 400, 235, WHITE);
  buffer_textout_centre_ex(normal, "Screen mode change takes effect after the game is restarted", 400, 285, LGREY);
  buffer_textout_centre_ex(normal, "Mouse Speed (Higher value means slower mouse)(Full screen only)", 400, 355, WHITE);
	buffer_textout_ex(normal, "Network name:", 320, 200, WHITE);
  sprintf(tempchar, "%-1d", global.mspeed);
  buffer_textout_centre_ex(normal, tempchar, 400, 375, WHITE);

  back.check();
  fxvol[0].check();
  fxvol[1].check();
  musvol[0].check();
  musvol[1].check();
  testsound.check();
  scroll.check();
  speed[0].check();
  speed[1].check();
  screenmode.check();
  windowres.check();
  part.check();
  arrange.check();
  showbattle.check();
	text.check();
	
  if (arrange.clicked())
  {
    global.maporder++;
    if (global.maporder >= 3) global.maporder = 0;
    switch(global.maporder)
    {
      case 0:
        sprintf(c, "Name");
        break;
      case 1:
        sprintf(c, "Number of Players");
        break;
      case 2:
        sprintf(c, "Map Size");
        break;
    }
    arrange.init(330, 130, 140, 20, c);
  }
  if (windowres.clicked())
  {
    if (global.windowed_resolution == 0)
    {
      windowres.init(295, 325, 210, 20, "Windowed Resolution: 1024x768");
      global.windowed_resolution = 1;
    }
    else if (global.windowed_resolution == 1)
    {
      windowres.init(295, 325, 210, 20, "Windowed Resolution: 1280x960");
      global.windowed_resolution = 2;
    }
    else
    {
      windowres.init(295, 325, 210, 20, "Windowed Resolution: 800x600");
      global.windowed_resolution = 0;
    }
  }
  if (part.clicked())
  {
    if (global.particles == 1)
    {
      global.particles = 0;
      part.init(350, 250, 100, 20, "Particles: OFF");
    }
    else
    {
      global.particles = 1;
      part.init(350, 250, 100, 20, "Particles: ON");
    }
  }
  if (screenmode.clicked())
  {
    if (global.fullscreen == 1)
    {
      global.fullscreen = 0;
      screenmode.init(295, 300, 100, 20, "Windowed");
    }
    else
    {
      global.fullscreen = 1;
      screenmode.init(295, 300, 100, 20, "Fullscreen");
    }
  }
  if (testsound.clicked())
  {
    play_sound(rand()%11);
  }
  if (fxvol[0].mousedown())
  {
    if (global.volume > 0) global.volume--;
    set_sfx_volume(global.volume);
  }
  if (fxvol[1].mousedown())
  {
    if (global.volume < 100) global.volume++;
    set_sfx_volume(global.volume);
  }
  if (musvol[0].mousedown())
  {
    if (global.music_volume > 0)
    {
      global.music_volume--;
      set_music_volume(global.music_volume);
    }
  }
  if (musvol[1].mousedown())
  {
    if (global.music_volume < 100)
    {
      global.music_volume++;
      set_music_volume(global.music_volume);
    }
  }
  if (scroll.clicked())
  {
    if (global.scroll_divider > 1)
    {
      global.scroll_divider >>= 1;  //divide by two, but I wanted to try using a bitshift operator
    }
    else
    {
      global.scroll_divider = 16;
    }
    switch(global.scroll_divider)
    {
      case 1:
        scroll.init(280, 80, 240, 20, "Map Scroll Speed: Very Fast");
        break;
      case 2:
        scroll.init(280, 80, 240, 20, "Map Scroll Speed: Fast");
        break;
      case 4:
        scroll.init(280, 80, 240, 20, "Map Scroll Speed: Normal");
        break;
      case 8:
        scroll.init(280, 80, 240, 20, "Map Scroll Speed: Slow");
        break;
      case 16:
        scroll.init(280, 80, 240, 20, "Map Scroll Speed: Slooooooooow");
        break;
    }
  }
  if (speed[0].clicked())
  {
    if (global.mspeed > 1) global.mspeed--;
    set_mouse_speed(global.mspeed, global.mspeed);
  }
  if (speed[1].clicked())
  {
    if (global.mspeed < 9) global.mspeed++;
    set_mouse_speed(global.mspeed, global.mspeed);
  }
  if (showbattle.clicked())
  {
    if (global.battle_animation == 0)
    {
      global.battle_animation = 1;
      showbattle.init(350, 170, 100, 20, "ON");
    }
    else
    {
      global.battle_animation = 0;
      showbattle.init(350, 170, 100, 20, "OFF");
    }
  }
	
  if (back.clicked())
  {
		global.netname = text.current_text();
		if (global.netname == "") global.netname = "Default";
    first = 1;
    if (ingame)
    {
      return 0;  //returning 0 means the player is going back to the game
    }
    else
    {
      return MENU_FIRST;
    }
  }
  draw_mouse();
  if (ingame)
  {
    return 1;  //returning 1 means this menu is still being used
  }
  else
  {
    return MENU_GAME_OPTIONS;
  }
}
