#ifndef _fileselect_h
#define _fileselect_h

class _fileselect
{  // this doesn't show hidden files
	_listbox dir, file;
	_button ok, cancel;
	char path[300];
	char **extensions;  // extensions to show (if NULL, everything's shown)
	char *title;
	short titlepos;  // store the title's x value, because centering it each time is too expensive
	short extnum;
	// no x, y, w, h parameters
public:
	void init(const char *_title = "", string ext = "");
	string check();  // returns "/NOTHING YET/" while still working, "/CANCEL/" if canceled
	void changedir(const char *newpath);
	void draw();
	void clear();
	char *getpath();

	_fileselect();
	~_fileselect();
};

typedef struct _extlist
{
	_listbox *l;
	char ***e;
	short n;
} _extlist;

int add_to_listbox(const char *filename, int attrib, void *param);

#endif
