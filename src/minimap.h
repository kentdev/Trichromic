#ifndef _minimap_h
#define _minimap_h

typedef struct _minimap
{
  ol::Bitmap *bmp;
  bool draw_called;
  int l, h, time;
  float a;
  void check();
  void create(bool defaultcolors = false);
  void draw(bool mapeditor, int x = 1, int y = 1, bool outline = true, bool defaultcolors = false);
  void destroy();
} _minimap;

#endif
