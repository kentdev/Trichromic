#ifndef _button_h
#define _button_h

class _button
{
	bool click, rclick;
	bool sound;
	int type;
	int textw;
	float texta;
	string text;
 public:
	bool clickable;
	short x;
	short y;
	short w;
	short h;
	int color;
	string get_text();
	ol::Bitmap *picture;
	void draw_disabled();
	void draw();
	void check();
	bool mouseover();
	bool mousedown();
	bool rmousedown();
	bool clicked();
	bool rclicked();
	void init(int ix, int iy, int iw, int ih, int itype = 0);
	void init(int ix, int iy, int iw, int ih, string itext, int itype = 0, int color = makecol(255,255,255));
	void init(int ix, int iy, int iw, int ih, string itext, ol::Bitmap *ipicture, int itype = 0, int color = makecol(0,0,0));
};

#endif
