#include "empire.h"

bool _button::mouseover()
{
	int mx;
	int my;
	mx = mouse_x;
	my = mouse_y;
	switch(type)
	{
		case 3:  //large hexagon
			if ((mx > x + 60) && (mx < x + 175))
			{
				if ((my >= y) && (my <= y + 200))
				{
					return true;
				}
			}
			else if ((mx >= x) && (mx <= x + 60))
			{
				if ((my >= y) && (my < y + 100))
				{
					if (my - y > 100 - ((mx - x) * (10.0 / 6.0))) return true;
				}
				else if ((my >= y + 100) && (my <= y + 200))
				{
					if (my - y - 100 < (mx - x) * (10.0 / 6.0)) return true;
				}
			}
			else if ((mx >= x + 175) && (mx <= x + 230))
			{
				if ((my >= y) && (my < y + 100))
				{
					if (my - y > (mx - x - 172) * (100.0 / 63.0)) return true;
				}
				else if ((my >= y + 100) && (my <= y + 200))
				{
					if (my - y - 100 < 100 - (mx - x - 175) * (10.0 / 6.0)) return true;
				}
			}
			break;
		case 4:  //small hexagon
			if ((mx > x + 31) && (mx < x + 100))
			{
				if ((my >= y) && (my <= y + 115))
				{
					return true;
				}
			}
			else if ((mx >= x) && (mx <= x + 31))
			{
				if ((my >= y) && (my < y + 56))
				{
					if (my - y > 56 - ((mx - x) * (57.0 / 31.0))) return true;
				}
				else if ((my >= y + 56) && (my <= y + 113))
				{
					if (my - y - 56 < (mx - x) * (57.0 / 31.0)) return true;
				}
			}
			else if ((mx >= x + 100) && (mx <= x + 165))
			{
				if ((my >= y) && (my < y + 56))
				{
					if (my - y > (mx - x - 100) * (57.0 / 31.0)) return true;
				}
				else if ((my >= y + 56) && (my <= y + 113))
				{
					if (my - y - 56 < 56 - (mx - x - 100) * (57.0 / 31.0)) return true;
				}
			}
			break;
		default:
			if ((mx >= x) && (mx <= x + w))
			{
				if ((my >= y) && (my <= y + h))
				{
					return true;
				}
			}
			break;
	}
	return false;
}

bool _button::mousedown()
{
	if (mouseover())
	{
		if (mouse_b & 1)
		{
			return true;
		}
	}
	return false;
}

bool _button::rmousedown()
{
	if (mouseover())
	{
		if (mouse_b & 2) return true;
	}
	return false;
}

bool _button::clicked()
{
	if (clickable)
	{
		if (click)
		{
			if ((!mousedown()) && (mouseover()))
			{
				click = false;
				return true;
			}
			else if (!(mouse_b & 1) && (!mouseover()))
			{
				click = false;
				return false;
			}
		}
		else
		{
			if (mousedown())
			{
				click = true;
			}
		}
	}
	return false;
}

bool _button::rclicked()
{
	if (clickable)
	{
		if (rclick)
		{
			if (!rmousedown() && mouseover())
			{
				rclick = false;
				return true;
			}
			else if (!(mouse_b & 2) && !mouseover())
			{
				rclick = false;
				return false;
			}
		}
		else
		{
			if (rmousedown()) rclick = true;
		}
	}
	return false;
}

void _button::draw_disabled()
{
	if ((picture == NULL) || (!picture->IsValid()))
	{
		buffer_rectrounded(x, y, x + w, y + h, GREY, 0.4);
		buffer_textout_ex(normal, text, x + (w / 2) - (textw / 2), y + (h / 2) - 5, DGREY);
	}
	else
	{
		buffer_draw_tinted_sprite(picture, x, y, ol::Rgba(0.5, 0.5, 0.5, 0.5));
	}
}

void _button::draw()
{
	if ((picture == NULL) || (!picture->IsValid()))
	{
		if (!mousedown() && !rmousedown())
		{
			switch(type)
			{
				case 0:  //standard button
					if (mouseover())
					{
						buffer_rectrounded(x, y, x + w, y + h, WHITE, 0.7);
					}
					else
					{
						buffer_rectrounded(x, y, x + w, y + h, WHITE, 0.4);
					}
					buffer_textout_ex(normal, text, x + (w / 2) - (textw / 2), y + (h / 2) - 5, BLACK);
					break;
				case 1:  //larger text, no background
					buffer_textout_ex(big, text, x, y, color, texta);
					break;
				case 2:  //colored rectangle
					buffer_rectfill(x, y, x + w, y + h, color);
					break;
				case 3:  //large hexagon
					if (mouseover())
					{
						buffer_draw_sprite(glb.bmp(33), x, y);
					}
					break;
				case 4:  //small hexagon
					if (mouseover())
					{
						buffer_draw_sprite(glb.bmp(34), x, y);
					}
					break;
				case 5:  //standard button, but with medium font and a little more opacity
					if (mouseover())
					{
						buffer_rectrounded(x, y, x + w, y + h, LGREY, 0.9);
					}
					else
					{
						buffer_rectrounded(x, y, x + w, y + h, WHITE, 0.9);
					}
					buffer_textout_ex(med, text, x + (w / 2) - (textw / 2), y + (h / 2) - 9, BLACK);
					break;
				case 6:
					if (mouseover())
					{
						buffer_rectfill(x, y, x + w, y + h, color, 0.3);
					}
					break;
			}
		}
    else  //if mousedown
    {
      switch(type)
      {
        case 0:
          buffer_rectrounded(x, y, x + w, y + h, WHITE, 0.7);
          buffer_textout_ex(normal, text, x + (w / 2) - (textw / 2) + 1, y + (h / 2) - 4, BLACK);
          break;
        case 1:
          buffer_textout_ex(big, text, x, y, color, texta);
          break;
				case 2:
					buffer_rectfill(x, y, x + w, y + h, color);
					break;
				case 3:  //large hexagon
					if (mouseover())
					{
						buffer_draw_sprite(glb.bmp(33), x, y);
					}
					break;
				case 4:  //small hexagon
					if (mouseover())
					{
						buffer_draw_sprite(glb.bmp(34), x, y);
					}
					break;
				case 5:
					buffer_rectrounded(x, y, x + w, y + h, WHITE, 0.9);
					buffer_textout_ex(med, text, x + (w / 2) - (textw / 2) + 1, y + (h / 2) - 8, BLACK);
					break;
				case 6:
					buffer_rectfill(x, y, x + w, y + h, color, 0.5);
					break;
      }
    }
  }
  else
  {
    buffer_blit_clipped(picture, x, y, 0, 0, w, h);
  }
}

void _button::check()
{
  draw();
  if (type == 1)
  {
    if (mouseover())
    {
      if (texta < 1.0)
      {
        texta += 0.05;
      }
    }
    else
    {
      if (texta > 0.5)
      {
        texta -= 0.05;
      }
    }
  }
  if (clickable)
  {
    if (mousedown() || rmousedown())
    {
      if (!sound)
      {
        play_sound(S_CLICK);
        sound = true;
      }
    }
    else
    {
      sound = false;
    }
  }
}

void _button::init(int ix, int iy, int iw, int ih, int itype)
{
  init(ix, iy, iw, ih, "", NULL, itype, BLACK);
}

void _button::init(int ix, int iy, int iw, int ih, string itext, int itype, int icolor)
{
  init(ix, iy, iw, ih, itext, NULL, itype, icolor);
}

void _button::init(int ix, int iy, int iw, int ih, string itext, ol::Bitmap *ipicture, int itype, int icolor)
{
  x = ix;
  y = iy;
  w = iw;
  h = ih;
  type = itype;
  text = itext;
  if (type == 0)
  {
    textw = normal.Width(text);
    if (global.current_resolution == 1) textw = int(textw / 1.28);
    else if (global.current_resolution == 2) textw = int(textw / 1.6);
    texta = 1.0;
  }
  else if (type == 1)
  {
    textw = big.Width(text);
    w = textw + 5;
    if (global.current_resolution == 1) textw = int(textw / 1.28);
    else if (global.current_resolution == 2) textw = int(textw / 1.6);
    texta = 0.5;
  }
	else if (type == 5)
	{
		textw = med.Width(text);
		if (global.current_resolution == 1) textw = int(textw / 1.28);
		else if (global.current_resolution == 2) textw = int(textw / 1.6);
		texta = 1.0;
	}
  picture = ipicture;
	color = icolor;
  click = false;
  rclick = false;
  sound = false;
  clickable = true;
}

string _button::get_text()
{
  return text;
}
