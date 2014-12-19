#include "empire.h"
using namespace ol;

extern int datafile_size(DATAFILE *d);

vector<drawthing> drawqueue;

void draw_all(ol::Bitmap *target)
{
	draw_all(false);
	copy_screen_to(target);
}

void draw_all(bool refreshcanvas)
{
  int i = 0;
  int s = drawqueue.size();
	int temp;
  drawthing t;
	Canvas::Fill(Rgba(BLACK, 255));
  while (i < s)
  {
    t = drawqueue[i];
    switch (t.type)
    {
      case DRAW_SPRITE:
        t.bmp->Blit(float(t.x), float(t.y), t.alpha);
        break;
      /*case DRAW_SPRITE_H_FLIP:
        t.bmp->Blit(float(t.x), float(t.y), FlippedMode(HORIZONTAL), t.alpha);
        break;*/
      case DRAW_TINTED_SPRITE:
        t.bmp->Blit(float(t.x), float(t.y), TintMode(t.colors[0]), t.alpha);
        break;
			/*case DRAW_TWOTINT_SPRITE:
				t.bmp->Blit(float(t.x), float(t.y), Tinted(t.colors[0]) + Tinted(t.colors[1]), t.alpha);
				break;*/
      case ROTATE_SPRITE:
        t.bmp->BlitRotated(float(t.x), float(t.y), float(t.angle) * 2 * AL_PI / 255, t.alpha);
        break;
			case ROTATE_TINTED_SPRITE:
				t.bmp->BlitRotated(float(t.x), float(t.y), float(t.angle) * 2 * AL_PI / 255, Tinted(t.colors[0]), t.alpha);
			  break;
			/*case ROTATE_TWOTINT_SPRITE:
				t.bmp->BlitRotated(float(t.x), float(t.y), float(t.angle) * 2 * AL_PI / 255, Tinted(t.colors[0]) + Tinted(t.colors[1]), t.alpha);
				break;*/
      case STRETCH_SPRITE:
        t.bmp->BlitStretched(float(t.x), float(t.y), float(t.w), float(t.h), t.alpha);
        break;
      case BLIT_CLIPPED:
        t.bmp->Blit(float(t.x - t.cx), float(t.y), ClippedMode(float(t.cx), float(t.cy), float(t.w), float(t.h)), t.alpha);
        break;
      case TEXTOUT_EX:
      	if (global.current_resolution == 1)
      	{
      		t.x = int(t.x * 1.28);
      		t.y = int(t.y * 1.28);
      	}
      	else if (global.current_resolution == 2)
      	{
      		t.x = int(t.x * 1.6);
      		t.y = int(t.y * 1.6);
      	}
        t.fnt->SetColor(Rgba(t.color, int(t.alpha * 255)));
        t.fnt->Print(t.str, t.x, t.y + t.fnt->Height(t.str) - 1);
        break;
      case TEXTOUT_CENTRE_EX:
        if (global.current_resolution == 1)
      	{
      		t.x = int(t.x * 1.28);
      		t.y = int(t.y * 1.28);
      	}
      	else if (global.current_resolution == 2)
      	{
      		t.x = int(t.x * 1.6);
      		t.y = int(t.y * 1.6);
      	}
        t.fnt->SetColor(Rgba(t.color, int(t.alpha * 255)));
        t.fnt->Print(t.str, t.x - (t.fnt->Width(t.str)) / 2, t.y + t.fnt->Height(t.str) - 1);
        break;
      case TEXTOUT_RIGHT_EX:
        if (global.current_resolution == 1)
      	{
      		t.x = int(t.x * 1.28);
      		t.y = int(t.y * 1.28);
      	}
      	else if (global.current_resolution == 2)
      	{
      		t.x = int(t.x * 1.6);
      		t.y = int(t.y * 1.6);
      	}
        t.fnt->SetColor(Rgba(t.color, int(t.alpha * 255)));
        t.fnt->Print(t.str, t.x - (t.fnt->Width(t.str)), t.y + t.fnt->Height(t.str) - 1);
        break;
			case CLIPPED_TEXTOUT_EX:
				if (global.current_resolution == 1)
      	{
      		t.x = int(t.x * 1.28);
      		t.y = int(t.y * 1.28);
      		t.w = int(t.w * 1.28);
      	}
      	else if (global.current_resolution == 2)
      	{
      		t.x = int(t.x * 1.6);
      		t.y = int(t.y * 1.6);
      		t.w = int(t.w * 1.6);
      	}
				temp = t.fnt->Height(t.str);
				Canvas::SetClipping(t.x, t.y, t.w, temp + 5);
				t.fnt->SetColor(Rgba(t.color, int(t.alpha * 255)));
				t.fnt->Print(t.str, t.x, t.y + temp - 1);
				Canvas::DisableClipping();
				break;
			case MULTILINE_TEXTOUT_EX:
				if (global.current_resolution == 1)
      	{
      		t.x = int(t.x * 1.28);
      		t.y = int(t.y * 1.28);
      		t.w = int(t.w * 1.28);
      	}
      	else if (global.current_resolution == 2)
      	{
      		t.x = int(t.x * 1.6);
      		t.y = int(t.y * 1.6);
      		t.w = int(t.w * 1.6);
      	}
				t.fnt->SetColor(Rgba(t.color, int(t.alpha * 255)));
        t.fnt->Print(t.str, t.x, t.y + t.fnt->Height("htpl") - 1, t.w);
				break;
      case CIRCLEFILL:
			  Circle (Vec2D(float(t.x), float(t.y)), t.r).Draw(Rgba(t.color, int(t.alpha * 255.0)));
        break;
			case ADDITIVE_CIRCLEFILL:
				Blenders::Set(ADDITIVE_BLENDER);
				Circle (Vec2D(float(t.x), float(t.y)), t.r).Draw(Rgba(t.color, int(t.alpha * 255.0)));
				Blenders::Set(ALPHA_BLENDER);
        break;
      case CIRCLEFILL_GRADIENT:
			  Circle (Vec2D(float(t.x), float(t.y)), t.r).DrawDisk(t.colors[0], t.colors[1], 0);
        break;
      case RECT:
			  Rect (float(t.x), float(t.y), float(t.w), float(t.h)).DrawOutline(Rgba(t.color, int(t.alpha * 255.0)));
        break;
      case RECTFILL:
			  Rect (float(t.x), float(t.y), float(t.w), float(t.h)).Draw(Rgba(t.color, int(t.alpha * 255.0)));
        break;
      case RECTFILL_GRADIENT:
			  Rect (float(t.x), float(t.y), float(t.w), float(t.h)).Draw(t.colors);
        break;
      case RECTROUNDED:
			  Rect (float(t.x), float(t.y), float(t.w), float(t.h), 1.0, t.r).Draw(Rgba(t.color, int(t.alpha * 255.0)));
        break;
      case MENUBACK:
			  Rect (float(t.x), float(t.y), float(t.w), float(t.h)).Draw(t.colors[1]);
			  Rect (float(t.x), float(t.y), float(t.w), float(t.h)).DrawOutline(t.colors[0]);
			  Rect (float(t.x + 1), float(t.y + 1), float(t.w - 2), float(t.h - 2)).DrawOutline(t.colors[2]);
        break;
      case PUTPIXEL:
			  Point (float(t.x), float(t.y)).Draw(Rgba(t.color, int(t.alpha * 255.0)));
        break;
      case MAPBLIP:
			  Point (float(t.x + 1), float(t.y + 1)).Draw(Rgba(WHITE, int(t.alpha * 255.0)));
			  Point (float(t.x + 3), float(t.y + 3)).Draw(Rgba(WHITE, int(t.alpha * 255.0)));
			  Point (float(t.x + 2), float(t.y + 2)).Draw(Rgba(t.color, int(t.alpha * 255.0)));
        break;
      case HLINE:
			  Line(float(t.x), float(t.y), float(t.x + t.w), float(t.y)).Draw(Rgba(t.color, int(t.alpha * 255.0)));
        break;
      case VLINE:
			  Line(float(t.x), float(t.y), float(t.x), float(t.y + t.h)).Draw(Rgba(t.color, int(t.alpha * 255.0)));
        break;
			case LINESTRIP:
				t.l->Draw(t.colors[0], t.colors[1]);
			  break;
      case CLEAR_TO_COLOR:
			  Canvas::Fill(Rgba(t.color, int(t.alpha * 255.0)));
        break;
			case SET_BLENDER_NORMAL:
				Blenders::Set(ALPHA_BLENDER);
				break;
			case SET_BLENDER_ADDITIVE:
				Blenders::Set(ADDITIVE_BLENDER);
				break;
    }
    i++;
  }
  normal.SetColor(Rgba(255, 255, 255));
  drawqueue.clear();
	
	if (refreshcanvas)
	{
	  Canvas::Refresh();
	}
}


int load_file(const char *filename, int attrib, void *param)
{
	((vector<string> *)param)->push_back(filename);
  return 0;
}
void bitmapdat::load(const char *path)
{
	Bitmap *bmp;
  string pattern;
	unsigned int i = 0;
  pattern = path;
	pattern = pattern + "/*.*";
	
  size = for_each_file_ex(pattern.c_str(), 0, FA_DIREC, load_file, &filenames);
	sort_by_name();
	
	while (i < filenames.size())
	{
		bmp = new Bitmap(filenames[i].c_str());
		b.push_back(bmp);
		empdebug(filenames[i]);
		i++;
	}
}

bitmapdat::bitmapdat()
{
	size = 0;
	name = "";
}

void bitmapdat::sort_by_name()
{
	string atemp;
	int i = 0, j, min, listsize = filenames.size();
	
	i = 0;
  while (i < listsize - 1)
  {  //  arrange filenames (the sort is kind of slow, but shouldn't take too long)
    min = i;
    j = i;
    while (j < listsize)
    {
      if (filenames[j] < filenames[min])
      {
        min = j;
      }
      j++;
    }
    if (min != i)
    {
      atemp = filenames[i];
      filenames[i] = filenames[min];
      filenames[min] = atemp;
    }
    i++;
  }
}

void bitmapdat::load(DATAFILE *d)
{
  int i = 0;
  ol::Bitmap *bmp;
  size = 0;
  while (d[i].type != DAT_END)
  {
    if (d[i].type == DAT_BITMAP)
    {
      size++;
    }
    i++;
  }
  
  i = 0;
  while (i < size)
  {
    bmp = new Bitmap();
    bmp->Load((BITMAP *)d[i].dat, CONVERT_MAGIC_PINK);
    b.push_back(bmp);
    i++;
  }
}

void bitmapdat::draw(int i, int x, int y, float a)
{
	if (i >= size)
	{
		empdebug("DRAWING ERROR:");
		char c[150];
		sprintf(c, "ERROR! Tried to access bitmap %d of %s, but size is %d", i, name.c_str(), size);
		empdebug(c);
		return;
	}
  buffer_draw_sprite(b[i], x, y, a);
}

ol::Bitmap *bitmapdat::bmp(int i)
{
	if (i >= size)
	{
		empdebug("DRAWING ERROR:");
		char c[150];
		sprintf(c, "ERROR! Tried to access bitmap %d of %s, but size is %d.  Using bitmap 0.", i, name.c_str(), size);
		empdebug(c);
		return b[0];
	}
  return b[i];
}

void bitmapdat::destroy()
{
  int i = 0;
  while (i < size)
  {
    b[i]->Destroy();
    delete b[i];
    i++;
  }
	b.clear();
}

void buffer_draw_sprite(ol::Bitmap *abitmap, int ax, int ay, float alpha)
{
  drawthing t;
  t.type = DRAW_SPRITE;
  t.x = ax;
  t.y = ay;
  t.bmp = abitmap;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_draw_tinted_sprite(ol::Bitmap *abitmap, int ax, int ay, ol::Rgba acolor, float alpha)
{
  drawthing t;
  t.type = DRAW_TINTED_SPRITE;
  t.bmp = abitmap;
  t.x = ax;
  t.y = ay;
  t.colors[0] = acolor;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_rotate_sprite(ol::Bitmap *abitmap, int ax, int ay, int aangle, float alpha)
{
  drawthing t;
  t.type = ROTATE_SPRITE;
  t.bmp = abitmap;
  t.x = ax;
  t.y = ay;
  t.angle = aangle;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_stretch_sprite(ol::Bitmap *abitmap, int ax, int ay, int aw, int ah, float alpha)
{
	drawthing t;
	t.type = STRETCH_SPRITE;
	t.bmp = abitmap;
	t.x = ax;
	t.y = ay;
	t.w = aw;
	t.h = ah;
	t.alpha = alpha;
	drawqueue.push_back(t);
}

void buffer_rotate_tinted_sprite(ol::Bitmap *abitmap, int ax, int ay, int aangle, Rgba acolor, float alpha)
{
	drawthing t;
	t.type = ROTATE_TINTED_SPRITE;
	t.bmp = abitmap;
	t.x = ax;
	t.y = ay;
	t.angle = aangle;
	t.alpha = alpha;
	t.colors[0] = acolor;
	drawqueue.push_back(t);
}

void buffer_blit_clipped(ol::Bitmap *abitmap, int ax, int ay, int sx, int sy, int aw, int ah, float alpha)
{
  drawthing t;
  t.type = BLIT_CLIPPED;
  t.x = ax;
  t.y = ay;
  t.cx = sx;
  t.cy = sy;
  t.w = aw;
  t.h = ah;
  t.bmp = abitmap;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_textout_ex(TextRenderer &afont, string amessage, int ax, int ay, int acolor, float alpha)
{
  drawthing t;
  t.type = TEXTOUT_EX;
  t.fnt = &afont;
  t.str = amessage;
  t.x = ax;
  t.y = ay;
  t.color = acolor;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_textout_centre_ex(TextRenderer &afont, string amessage, int ax, int ay, int acolor, float alpha)
{
  drawthing t;
  t.type = TEXTOUT_CENTRE_EX;
  t.fnt = &afont;
  t.str = amessage;
  t.x = ax;
  t.y = ay;
  t.color = acolor;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_textout_right_ex(TextRenderer &afont, string amessage, int ax, int ay, int acolor, float alpha)
{
  drawthing t;
  t.type = TEXTOUT_RIGHT_EX;
  t.fnt = &afont;
  t.str = amessage;
  t.x = ax;
  t.y = ay;
  t.color = acolor;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_circlefill(int ax, int ay, float ar, int acolor, float alpha)
{
  drawthing t;
  t.type = CIRCLEFILL;
  t.x = ax;
  t.y = ay;
  t.r = ar;
  t.color = acolor;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_additive_circlefill(int ax, int ay, float ar, int acolor, float alpha)
{
  drawthing t;
  t.type = ADDITIVE_CIRCLEFILL;
  t.x = ax;
  t.y = ay;
  t.r = ar;
  t.color = acolor;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_circlefill_gradient(int ax, int ay, float ar, Rgba inner, Rgba outer)
{
  drawthing t;
  t.type = CIRCLEFILL_GRADIENT;
  t.x = ax;
  t.y = ay;
  t.r = ar;
  t.colors[0] = inner;
  t.colors[1] = outer;
  drawqueue.push_back(t);
}

void buffer_rect(int ax, int ay, int bx, int by, int acolor, float alpha)
{
  drawthing t;
  t.type = RECT;
  t.x = ax;
  t.y = ay;
  t.w = bx - ax;
  t.h = by - ay;
  if (t.w < 0) t.w = 0;
  if (t.h < 0) t.h = 0;
  t.color = acolor;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_rectfill(int ax, int ay, int bx, int by, int acolor, float alpha)
{
  drawthing t;
  t.type = RECTFILL;
  t.x = ax;
  t.y = ay;
  t.w = bx - ax;
  t.h = by - ay;
  if (t.w < 0) t.w = 0;
  if (t.h < 0) t.h = 0;
  t.color = acolor;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_rectfill_gradient(int ax, int ay, int bx, int by, int tl, int tr, int br, int bl)
{
  drawthing t;
  t.type = RECTFILL_GRADIENT;
  t.x = ax;
  t.y = ay;
  t.w = bx - ax;
  t.h = by - ay;
  if (t.w < 0) t.w = 0;
  if (t.h < 0) t.h = 0;
  t.colors[0] = Rgba(tl, 255);
  t.colors[1] = Rgba(tr, 255);
  t.colors[2] = Rgba(br, 255);
  t.colors[3] = Rgba(bl, 255);
  drawqueue.push_back(t);
}

void buffer_rectrounded(int ax, int ay, int bx, int by, int acolor, float alpha, float roundness)
{
  drawthing t;
	t.type = RECTROUNDED;
  t.x = ax;
  t.y = ay;
  t.w = bx - ax;
  t.h = by - ay;
	t.r = roundness;
  if (t.w < 0) t.w = 0;
  if (t.h < 0) t.h = 0;
  t.color = acolor;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_menuback(int ax, int ay, int bx, int by, int acolor1, int acolor2, int acolor3, float alpha)
{
  drawthing t;
  t.type = MENUBACK;
  t.x = ax;
  t.y = ay;
  t.w = bx - ax;
  t.h = by - ay;
  if (t.w < 0) t.w = 0;
  if (t.h < 0) t.h = 0;
  t.colors[0] = Rgba(acolor1, int(alpha * 255.0));
  t.colors[1] = Rgba(acolor2, int(alpha * 255.0));
  t.colors[2] = Rgba(acolor3, int(alpha * 255.0));
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_putpixel(int ax, int ay, int acolor, float alpha)
{
  drawthing t;
  t.type = PUTPIXEL;
  t.x = ax;
  t.y = ay;
  t.color = acolor;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_mapblip(int ax, int ay, int acolor, float alpha)
{
  drawthing t;
  t.type = MAPBLIP;
  t.x = ax;
  t.y = ay;
  t.color = acolor;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_hline(int ax, int ay, int bx, int acolor, float alpha)
{
  drawthing t;
  t.type = HLINE;
  t.x = ax;
  t.y = ay;
  t.w = bx - ax;
  if (t.w < 0)
  {
    t.x = bx;
    t.w = ax - bx;
  }
  t.color = acolor;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_vline(int ax, int ay, int by, int acolor, float alpha)
{
  drawthing t;
  t.type = VLINE;
  t.x = ax;
  t.y = ay;
  t.h = by - ay;
  if (t.h < 0)
  {
    t.y = by;
    t.h = ay - by;
  }
  t.color = acolor;
  t.alpha = alpha;
  drawqueue.push_back(t);
}

void buffer_linestrip(LineStrip *al, Rgba startcolor, Rgba endcolor)
{
	drawthing t;
	t.type = LINESTRIP;
	t.l = al;
	t.colors[0] = startcolor;
	t.colors[1] = endcolor;
	drawqueue.push_back(t);
}

void buffer_clear_to_color(int acolor)
{
  drawthing t;
  t.type = CLEAR_TO_COLOR;
  t.color = acolor;
  drawqueue.push_back(t);
}

void clipped_textout_ex(TextRenderer &f, string text, int x, int y, int l, int c, float alpha)
{
	drawthing t;
	t.type = CLIPPED_TEXTOUT_EX;
	t.str = text;
	t.fnt = &f;
	t.x = x;
	t.y = y;
	t.w = l;
	t.color = c;
	t.alpha = alpha;
	drawqueue.push_back(t);
}

void multiline_textout_ex(TextRenderer &f, string text, int x, int y, int l, int c, float alpha)
{
	drawthing t;
	t.type = MULTILINE_TEXTOUT_EX;
	t.str = text;
	t.fnt = &f;
	t.x = x;
	t.y = y;
	t.w = l;
	t.color = c;
	t.alpha = alpha;
	drawqueue.push_back(t); //To be uncommented when the TextRenderer->Width() function works better
	
	// The following is here here because TextRenderer->Width() takes WAY TOO
	// MUCH processing power.  These function should be removed once Glyph
	// Keeper / OpenLayer improve Width().
	
	/*static string prevtext = "";
	static TextRenderer *prevfont = NULL;
	static int *wordpos = NULL;
	static int *wordlen = NULL;
	static int spacelen = 0;
	static int size = 0;
	bool recalc = false;
  int i, h, t, last;
  string line;
	
	if (text == "/GAMEOVER/")
	{
		if (wordpos != NULL)
		{
			free(wordpos);
			wordpos = NULL;
		}
		if (wordlen != NULL)
		{
			free(wordlen);
			wordlen = NULL;
		}
		return;
	}
	
	if (text.length() == 0) return;
	
	if ((prevtext != text) || (prevfont != &f))
	{
		recalc = true;
	}
	
	if (recalc)
	{
		prevtext = text;
		prevfont = &f;
		if (wordpos != NULL)
		{
			free(wordpos);
			wordpos = NULL;
		}
		if (wordlen != NULL)
		{
			free(wordlen);
			wordlen = NULL;
		}
		
		spacelen = f.Width(" ");
		size = 0;
		i = 1;
		last = 0;
		while (unsigned(i) < text.length())
		{
			if ((text.substr(i, 1) == " ") || (unsigned(i) == text.length() - 1))
			{
				size++;
				wordpos = (int*)realloc(wordpos, sizeof(int) * size);
				wordlen = (int*)realloc(wordlen, sizeof(int) * size);
				wordpos[size - 1] = i;
				wordlen[size - 1] = f.Width(text.substr(last, i - last));
				last = i + 1;
			}
			i++;
		}
		recalc = false;
	}
	
	i = 0;
	t = 0;
	h = 0;
	last = 0;
	while (i < size - 1)
	{
		t += wordlen[i];
		if (t + spacelen + wordlen[i + 1] >= l)
		{
			line = text.substr(last, wordpos[i] - last);
			last = wordpos[i] + 1;
			buffer_textout_ex(f, line, x, y + h, c, alpha);
			h += f.Height(line);
			t = 0;
		}
		else
		{
			t += spacelen;
		}
		i++;
	}
	line = text.substr(last, text.length() - last);
	buffer_textout_ex(f, line, x, y + h, c, alpha);*/
}

void copy_screen_to(ol::Bitmap *target)
{
	ol::Bitmap temp(global.screen_width, global.screen_height);
	temp.CopyFromScreen(0, 0);

	Canvas::SetTo(*target);
	Settings::RestoreOldProjection();
//	Transforms::SetPosition(0, 1);
	Canvas::Fill(Rgba(BLACK, 255));
	temp.BlitStretched(0, 0, 800, 600);
	Canvas::Refresh();
	Canvas::SetTo(SCREEN_BACKBUF);
	Settings::SetOrthographicProjection(800, 600);
	
	temp.Destroy();
}

void set_normal_blender()
{
	drawthing t;
	t.type = SET_BLENDER_NORMAL;
	drawqueue.push_back(t);
}

void set_additive_blender()
{
	drawthing t;
	t.type = SET_BLENDER_ADDITIVE;
	drawqueue.push_back(t);
}
