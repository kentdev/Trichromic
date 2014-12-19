#include "empire.h"

_fileselect::_fileselect()
{
	extensions = NULL;
	title = NULL;
}

_fileselect::~_fileselect()
{
	clear();
}

void _fileselect::clear()
{
	int i = 0;
	
	if (title != NULL)
	{
		free(title);
		title = NULL;
	}
	
	if (extensions != NULL)
	{
		while (i < extnum)
		{
			free(extensions[i]);
			extensions[i] = NULL;
			i++;
		}
		free(extensions);
		extensions = NULL;
	}
}

void _fileselect::init(const char *t, string ext)
{
	char *s, c[50], d[50];
	int i;
	
	get_executable_name(path, 300);
	replace_filename(path, path, "", 300);
	put_backslash(path);
	
	dir.init(40, 100, 275, 295);
	file.init(325, 100, 275, 295);
	ok.init(500, 410, 100, 20, "OK");
	cancel.init(40, 410, 100, 20, "Cancel");
	
	if (t != NULL)
	{
		title = (char*)malloc(sizeof(char) * (strlen(t) + 10));
		titlepos = 320 - (normal.Width(t) / 2);
		strcpy(title, t);
	}
	
	if (ext == "")
	{
		extensions = NULL;
		extnum = 0;
	}
	else
	{
		strcpy(c, ext.c_str());
		strcpy(d, c);
		extensions = (char**)malloc(sizeof(char*));
		extnum = 0;
		s = strtok(c, ";");
		while (s != NULL)
		{
			extnum++;
			s = strtok(NULL, ";");
		}
		extensions = (char**)malloc(sizeof(char *) * extnum);

		s = strtok(d, ";");
		i = 0;
		while (s != NULL)
		{
			extensions[i] = (char*)malloc(sizeof(char) * (strlen(s) + 1));
			strcpy(extensions[i], s);
			s = strtok(NULL, ";");
			i++;
		}
	}
	
	changedir("");
}

void _fileselect::changedir(const char *addition)
{
	char temp[300];
	_extlist extlist;
	
	make_absolute_filename(path, path, addition, 300);
	put_backslash(path);
	
	strcpy(temp, path);
	strcat(temp, "*");  //  the result is '/the/current/path/*'
	
	dir.clear();
	file.clear();
	
	extlist.l = &dir;
	extlist.e = &extensions;
	extlist.n = 0;  //slightly hackish: this way the extension filter doesn't filter folders
	for_each_file_ex(temp, FA_DIREC, FA_HIDDEN, add_to_listbox, &extlist);
	dir.sort();
	
	extlist.l = &file;
	extlist.n = extnum;
	for_each_file_ex(temp, 0, FA_DIREC | FA_HIDDEN, add_to_listbox, &extlist);
	file.sort();
}

string _fileselect::check()
{
	char *t = NULL;
	string s;
	
	buffer_rectfill(30, 55, 610, 440, DGREY);
	buffer_rect(30, 55, 610, 440, LGREY);
	buffer_textout_ex(normal, title, titlepos, 60, WHITE);
	clipped_textout_ex(normal, path, 50, 77, 550, LGREY);
	
	dir.check();
	file.check();
	ok.check();
	cancel.check();
	
	if (dir.getsel() != -1)
	{
		t = (char*)malloc(sizeof(char) * (strlen(dir.selected()) + 2));
		strcpy(t, dir.selected());
		strcat(t, "/");
		changedir(t);
		free(t);
	}
	
	if (file.getsel() != -1)
	{
		if (ok.clicked())
		{
			t = (char*)malloc(sizeof(char) * (strlen(path) + strlen(file.selected()) + 1));
			strcpy(t, path);
			strcat(t, file.selected());
			s = t;
			free(t);
			return s;
		}
	}
	
	if (cancel.clicked())
	{
		return "/CANCEL/";
	}
	
	return "/NOTHING YET/";
}

int add_to_listbox(const char *filename, int attrib, void *param)
{
	char *fname = get_filename(filename);
	char *ext = get_extension(fname);
	_extlist extlist = *((_extlist *)param);
	short extnum = extlist.n;
	int i = 0;
	
	if (extnum > 0)
	{  //  if extensions are specified, only show files with those extensions
		while (i < extnum)
		{
			if (stricmp(ext, (*extlist.e)[i]) == 0)
			{
				(extlist.l)->additem(fname);
				return 0;
			}
			i++;
		}
		return 0;
	}
	
	(extlist.l)->additem(fname);
	return 0;
}
