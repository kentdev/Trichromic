#include "empire.h"

int mouse_type;

void reset_mouse_counter();
void check_mouse_stuff();
void draw_mouse();
int mouse_clicked();
int mouse_rclicked();

bool new_logic_frame, clicked, rclicked;

void reset_mouse_counter()
{
	new_logic_frame = true;
}

void check_mouse_stuff()
{
	static bool mousedown = false;
	static bool mouserdown = false;
	
	clicked = false;
	rclicked = false;
	
	if (mouse_b & 1)
	{
		mousedown = true;
	}
	else
	{
		if (mousedown)
		{
			mousedown = false;
			clicked = true;
		}
	}
	
	if (mouse_b & 2)
	{
		mouserdown = true;
	}
	else
	{
		if (mouserdown)
		{
			mouserdown = false;
			rclicked = true;
		}
	}
}

int mouse_clicked()
{
	return clicked;
}

int mouse_rclicked()
{
	return rclicked;
}

void draw_mouse(int frame)
{
  mouse.draw(frame, mouse_x, mouse_y, 0.9);
}
