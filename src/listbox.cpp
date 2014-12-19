#include "empire.h"

const int listbox_textheight = 12;
const int listbox_scrollbarwidth = 10;

_listbox::_listbox()
{
	listsize = 0;
	sel = -1;
	list = NULL;
}

_listbox::~_listbox()
{
	clear();
}

void _listbox::init(int _x, int _y, int _w, int _h)
{
	clear();
	x = _x;
	y = _y;
	w = _w - listbox_scrollbarwidth;
	h = _h;
	listsize = 0;
	sel = -1;
	offset = 0;
	list = NULL;
	up.init(x + w + 1, y - 1, listbox_scrollbarwidth, 10);
	down.init(x + w + 1, y + h - 9, listbox_scrollbarwidth, 10);
	mouse_zval = mouse_z;
}

void _listbox::additem(const char *text)
{
	char *item;
	
	if (text == NULL) return;
	
	item	= (char *)malloc(sizeof(char) * (strlen(text) + 1));
	strcpy(item, text);

	listsize++;
	if (list != NULL)
	{
		list = (char **)realloc(list, sizeof(char *) * listsize);
		list[listsize - 1] = item;
		return;
	}
	list = (char **)malloc(sizeof(char *));
	list[0] = item;
}

int _listbox::size()
{
	return listsize;
}

char *_listbox::getitem(int i)
{
	return list[i];
}

void _listbox::clear()
{
	int i = 0;
	
	listsize = 0;
	sel = -1;
	offset = 0;
	delay = 0;

	if (list == NULL) return;	
	while (i < listsize)
	{
		free(list[i]);
		list[i] = NULL;
		i++;
	}
	free(list);
	list = NULL;
}

void _listbox::check()
{
	if (delay <= 0)
	{
		if (up.mousedown())
		{
			scroll(-1);
			delay = 5;
		}
		else if (down.mousedown())
		{
			scroll(1);
			delay = 5;
		}
		else if ((sel != -1) && (key[KEY_UP]))
		{
			if ((mouse_x > x) && (mouse_x < x + w - 2))
			{
				if ((mouse_y > y) && (mouse_y < y + h))
				{
					if (sel > 0) sel--;
					if (sel < offset) scroll(-1);
					delay = 5;
				}
			}
		}
		else if ((sel != -1) && (key[KEY_DOWN]))
		{
			if ((mouse_x > x) && (mouse_x < x + w - 2))
			{
				if ((mouse_y > y) && (mouse_y < y + h))
				{
					if (sel < listsize - 1) sel++;
					if (sel > offset + (h / listbox_textheight) - 2) scroll(1);
					delay = 5;
				}
			}
		}
		else if (mouse_zval != mouse_z)
		{
			if (mouse_z > mouse_zval)
			{
				scroll(-1);
			}
			else
			{
				scroll(1);
			}
			mouse_zval = mouse_z;
		}
	}
	else
	{
		delay--;
	}
	
	if (mouse_clicked() == 1)
	{
		if ((mouse_x > x) && (mouse_x < x + w - 2))
		{
			if ((mouse_y > y) && (mouse_y < y + h))
			{
				sel = ((mouse_y - y) / listbox_textheight) + offset;
				if (sel >= listsize) sel = -1;
			}
		}
	}
	
	draw();
}

void _listbox::draw(float alpha, bool textonly, bool reverseorder)
{
	int i = 0;
	float bgalpha = alpha - 0.35;
	if (bgalpha < 0) bgalpha = 0;
	
	buffer_rectfill(x, y, x + w + listbox_scrollbarwidth, y + h, BLACK, bgalpha);
	
	if (!textonly)
	{
		buffer_rect(x, y, x + w, y + h, GREY);
	}
	
	
	while ((i + offset < listsize) && ((i + 1) * listbox_textheight < h))
	{
		if ((!textonly) && (i + offset == sel))
		{
			buffer_rectfill(x, y + listbox_textheight * i, x + w, y + listbox_textheight * (i + 1), GREY, 0.5);
		}
		if (reverseorder)
		{
			clipped_textout_ex(normal, list[listsize - 1 - i], x + 2, y + h - listbox_textheight * (i + 1), w - 3, WHITE, alpha);
		}
		else
		{
			clipped_textout_ex(normal, list[i + offset], x + 2, y + listbox_textheight * i, w - 3, WHITE, alpha);
		}
		i++;
	}
	
	if (!textonly)
	{
		buffer_vline(x + w + listbox_scrollbarwidth, y, y + h, GREY);
		up.check();
		down.check();
	}
}

void _listbox::scroll(int n)
{
	int itemslisted = h / listbox_textheight;
	
	offset += n;
	if (offset > listsize - itemslisted) offset = listsize - itemslisted;
	if (offset < 0) offset = 0;
}

void _listbox::scroll_to_end()
{
	int itemslisted = h / listbox_textheight;
	offset = listsize - itemslisted;
	if (offset < 0) offset = 0;
}

void _listbox::sort()
{
	char **temp = (char**)malloc(sizeof(char *) * listsize);
	char *atemp;
	int i = 0, j, min;
	
	while (i < listsize)
	{
		temp[i] = list[i];
		i++;
	}
	
	i = 0;
  while (i < listsize - 1)
  {  //  arrange filenames (the sort is kind of slow, but shouldn't take too long)
    min = i;
    j = i;
    while (j < listsize)
    {
      if (strcmp(temp[j], temp[min]) < 0)
      {
        min = j;
      }
      j++;
    }
    if (min != i)
    {
      atemp = temp[i];
      temp[i] = temp[min];
      temp[min] = atemp;
    }
    i++;
  }
	free(list);
	list = temp;
	
	counter = 0;  //but in case it does take a while, don't let it affect the program too badly
}

void _listbox::setsel(int i)
{
	sel = i;
}

char *_listbox::selected()
{
	if (sel == -1) return NULL;
	return list[sel];
}

int _listbox::getsel()
{
	return sel;
}

void _listbox::unselect()
{
	sel = -1;
}
