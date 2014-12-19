#ifndef _draw_h
#define _draw_h

#define DRAW_SPRITE            0
#define DRAW_TINTED_SPRITE     1
//#define DRAW_TWOTINT_SPRITE    2
//#define DRAW_SPRITE_H_FLIP     3
#define ROTATE_SPRITE          3
#define ROTATE_TINTED_SPRITE   4
//#define ROTATE_TWOTINT_SPRITE  5
#define STRETCH_SPRITE         6
#define BLIT_CLIPPED           8
#define TEXTOUT_EX             9
#define TEXTOUT_CENTRE_EX      10
#define CLIPPED_TEXTOUT_EX     11
#define MULTILINE_TEXTOUT_EX   12
#define CIRCLEFILL             13
#define ADDITIVE_CIRCLEFILL    14
#define CIRCLEFILL_GRADIENT    15
#define RECT                   16
#define RECTFILL               17
#define RECTFILL_GRADIENT      18
#define RECTROUNDED            19
#define MENUBACK               20
#define PUTPIXEL               21
#define MAPBLIP                22
#define HLINE                  23
#define VLINE                  24
#define LINESTRIP              25
#define CLEAR_TO_COLOR         26
#define COPY_TO                27
#define SET_BLENDER_NORMAL     28
#define SET_BLENDER_ADDITIVE   29
#define TEXTOUT_RIGHT_EX       30

typedef struct drawthing
{
  short type;
  short angle;  // 0 to 255
	int x, y, w, h, color, cx, cy;
  float alpha; // 0.0 to 1.0
  float r;
  ol::Rgba colors[4];
  string str;
  ol::Bitmap *bmp;
  ol::TextRenderer *fnt;
	ol::LineStrip *l;
} drawthing;

typedef struct bitmapdat
{
	bitmapdat();
  vector<ol::Bitmap*> b;
	vector<string> filenames;
  void load(DATAFILE *dat);
	void load(const char *path);
  void draw(int i, int x, int y, float a = 1.0);
	void sort_by_name();
  void destroy();
  string name;
  int size;
  ol::Bitmap *bmp(int i);
} bitmapdat;

extern vector<drawthing> drawqueue;

void buffer_draw_sprite(ol::Bitmap *abitmap, int ax, int ay, float alpha = 1.0);
void buffer_draw_twotint_sprite(ol::Bitmap *abitmap, int ax, int ay, ol::Rgba color1, ol::Rgba color2, float alpha = 1.0);
void buffer_draw_tinted_sprite(ol::Bitmap *abitmap, int ax, int ay, ol::Rgba color, float alpha = 1.0);
void buffer_rotate_sprite(ol::Bitmap *abitmap, int ax, int ay, int aangle, float alpha = 1.0);
void buffer_rotate_tinted_sprite(ol::Bitmap *abitmap, int ax, int ay, int aangle, ol::Rgba acolor, float alpha = 1.0);
void buffer_stretch_sprite(ol::Bitmap *abitmap, int ax, int ay, int aw, int ah, float alpha = 1.0);
void buffer_blit_clipped(ol::Bitmap *abitmap, int ax, int ay, int sx, int sy, int aw, int ah, float alpha = 1.0);

void buffer_textout_ex(ol::TextRenderer &afont, string amessage, int ax, int ay, int acolor, float alpha = 1.0);
void buffer_textout_centre_ex(ol::TextRenderer &afont, string amessage, int ax, int ay, int acolor, float alpha = 1.0);
void buffer_textout_right_ex(ol::TextRenderer &afont, string amessage, int ax, int ay, int acolor, float alpha = 1.0);
void clipped_textout_ex(ol::TextRenderer &f, string text, int x, int y, int l, int c, float alpha = 1.0);
void multiline_textout_ex(ol::TextRenderer &f, string text, int x, int y, int l, int c, float alpha = 1.0);
void buffer_circlefill(int ax, int ay, float ar, int acolor, float alpha = 1.0);
void buffer_additive_circlefill(int ax, int ay, float ar, int acolor, float alpha = 1.0);
void buffer_circlefill_gradient(int ax, int ay, float ar, ol::Rgba inner, ol::Rgba outer);
void buffer_rect(int ax, int ay, int bx, int by, int acolor, float alpha = 1.0);
void buffer_rectfill(int ax, int ay, int bx, int by, int acolor, float alpha = 1.0);
void buffer_rectfill_gradient(int ax, int ay, int bx, int by, int tl, int tr, int br, int bl);
void buffer_rectrounded(int ax, int ay, int bx, int by, int acolor, float alpha = 1.0, float roundness = 2.0);
void buffer_menuback(int ax, int ay, int bx, int by, int acolor1, int acolor2, int acolor3, float alpha = 1.0);
void buffer_putpixel(int ax, int ay, int acolor, float alpha = 1.0);
void buffer_mapblip(int ax, int ay, int acolor, float alpha = 1.0);
void buffer_hline(int ax, int ay, int bx, int acolor, float alpha = 1.0);
void buffer_vline(int ax, int ay, int by, int acolor, float alpha = 1.0);
void buffer_linestrip(ol::LineStrip *al, ol::Rgba startcolor, ol::Rgba endcolor);
void buffer_clear_to_color(int acolor);
void copy_screen_to(ol::Bitmap *target);
void set_normal_blender();
void set_additive_blender();

void draw_all(bool refreshcanvas = true);
void draw_all(ol::Bitmap *target);

#endif
