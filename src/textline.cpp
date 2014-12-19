#include "empire.h"

void _textline::init(int _x, int _y, int _w, int _h, bool _enter_clears)
{
	reset();
	x = _x;
	y = _y;
	w = _w;
	h = _h;
	enter_clears = _enter_clears;
}

void _textline::reset()
{
	edittext = "";
	iter = edittext.begin();
	caret = 0;
	insert = true;
}

void _textline::set_text(string text)
{
	edittext = text;
	caret = edittext.length();
	iter = edittext.end();
}

string _textline::current_text()
{
	return edittext;
}

string _textline::check(float bgalpha)
{  //textline code by 23yrold3yrold, just modified into a class
	char ASCII, scancode;
	int newkey, width;
	string returntext;
	
	while(keypressed())
  {
	  newkey = readkey();
		ASCII = newkey & 0xff;
		scancode = newkey >> 8;

		// a character key was pressed; add it to the string
		if(ASCII >= 32 && ASCII <= 126)
		{  // add the new char, inserting or replacing as need be
			if (insert || iter == edittext.end())
			{
				width = normal.Width(edittext + ASCII);
				if (global.current_resolution == 1) width = int(width / 1.28);
				if (global.current_resolution == 2) width = int(width / 1.6);
				
				if (width < w)
				{
					iter = edittext.insert(iter, ASCII);
					caret++;  // increment both the caret and the iterator
					iter++;
				}
			}
			else
			{
				edittext.replace(caret, 1, 1, ASCII);
				caret++;  // increment both the caret and the iterator
				iter++;
			}
		}
		else  // some other, "special" key was pressed; handle it here
		{
			switch(scancode)
			{
				case KEY_DEL:
					if (iter != edittext.end()) iter = edittext.erase(iter);
					break;
				case KEY_BACKSPACE:
					if(iter != edittext.begin())
					{
						caret--;
						iter--;
						iter = edittext.erase(iter);
					}
					break;
				case KEY_RIGHT:
					if(iter != edittext.end()) caret++, iter++;
					break;
				case KEY_LEFT:
					if(iter != edittext.begin()) caret--, iter--;
					break;
				case KEY_INSERT:
					if(insert) insert = 0; else insert = 1;
					break;
				case KEY_ENTER:
					if (enter_clears)
					{
						returntext = edittext;
						reset();
						return returntext;
					}
					break;
			}
		}
	}
	draw(bgalpha);
	
	return "/NOTHING YET/";
}

void _textline::draw(float bgalpha)
{
	int width = normal.Width(edittext.substr(0, caret));
	
	if (global.current_resolution == 1) width = int(width / 1.28);
	if (global.current_resolution == 2) width = int(width / 1.6);
	
	buffer_rectfill(x, y, x + w, y + h, DGREY, bgalpha);
	buffer_textout_ex(normal, edittext, x, y, WHITE);
	if (insert)
	{
		buffer_vline(x + width, y, y + h, WHITE);
	}
	else
	{
		buffer_rect(x + width, y, x + normal.Width(edittext.substr(0, caret + 1)), y + h, WHITE);
	}
}
