#ifndef _TEXTLINE_H
#define _TEXTLINE_H

class _textline
{
	bool enter_clears;
	bool insert;            // true of should text be inserted
	int caret;              // tracks the text caret
	int x, y, w, h;
	string edittext;        // an empty string for editting
	string::iterator iter;  // string iterator
public:
	void init(int _x, int _y, int _w, int _h, bool _enter_clears);
	void reset();
	void draw(float bgalpha = 0.7);
	void set_text(string text);
	string check(float bgalpha = 0.7);  //only returns something other than "/NOTHING YET/" if (enter_clears) and the user hits enter
	string current_text();
};

#endif
