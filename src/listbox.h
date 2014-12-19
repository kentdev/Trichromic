#ifndef _listbox_h
#define _listbox_h

class _listbox
{
	_button up, down;
	short x, y, w, h;
	short listsize;
	short sel;
	short offset;
	short delay;
	int mouse_zval;
	char **list;
	
public:
	void init(int x, int y, int w, int h);
	void additem(const char *item);
	void check();
	void unselect();
	void draw(float textopacity = 1.0, bool textonly = false, bool reverseorder = false);
	void clear();
	void sort();
	void scroll(int n);
	void scroll_to_end();
	void setsel(int i);
	char *selected();
	char *getitem(int i);
	int getsel();
	int size();

	_listbox();
	~_listbox();
};



#endif
