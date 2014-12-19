# Trichromic makefile

CC = g++
LD = g++
ifdef MINGDIR
CFLAGS = -Wall -c -D_REENTRANT -DDY_WINDOWS_VERSION
LDFLAGS = -lopenlayer -lglyph-agl -lfreetype -lldpng -lpng -lz -lagl -lalleg -luser32 -lgdi32 -lglu32 -lopengl32 -lfmodex -ldconn -lzlib1 -lws2_32 -mwindows
PROG = trichromic
else
CFLAGS = -Wall -c -pthread `openlayer-config --cflags`
LDFLAGS = `openlayer-config --libs` -lfmodex -lz -ldconn
PROG = trichromic_bin
endif
RM = rm -f
ifdef MINGDIR
OBJS = obj/ai.o obj/ai_action_attack.o obj/ai_action_other.o obj/ai_action_transport.o obj/ai_build_units.o \
  obj/ai_path.o obj/ai_types.o obj/attackables.o obj/battle.o obj/battleshot.o obj/button.o obj/draw.o \
	obj/draw_stats.o obj/empire.o obj/fileselect.o obj/gameoptions.o obj/ingamemenu.o obj/logic.o \
	obj/listbox.o obj/mainmenu.o obj/map.o obj/mapedit.o obj/maplist.o obj/mapunit.o obj/minimap.o \
	obj/mouse.o obj/moveselect.o obj/netplay.o obj/netudp.o obj/networkclient.o obj/networkhost.o \
	obj/networking.o obj/particle.o obj/player.o obj/playeredit.o obj/ranges.o obj/scrolling.o obj/sidebar.o \
	obj/sound.o obj/textline.o obj/units.o obj/unloadables.o res.o
else
OBJS = obj/ai.o obj/ai_action_attack.o obj/ai_action_other.o obj/ai_action_transport.o obj/ai_build_units.o \
  obj/ai_path.o obj/ai_types.o obj/attackables.o obj/battle.o obj/battleshot.o obj/button.o obj/draw.o \
	obj/draw_stats.o obj/empire.o obj/fileselect.o obj/gameoptions.o obj/ingamemenu.o obj/logic.o \
	obj/listbox.o obj/mainmenu.o obj/map.o obj/mapedit.o obj/maplist.o obj/mapunit.o obj/minimap.o \
	obj/mouse.o obj/moveselect.o obj/netplay.o obj/netudp.o obj/networkclient.o obj/networkhost.o \
	obj/networking.o obj/particle.o obj/player.o obj/playeredit.o obj/ranges.o obj/scrolling.o obj/sidebar.o \
	obj/sound.o obj/textline.o obj/units.o obj/unloadables.o
endif

$(PROG): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(PROG)


obj/%.o: src/%.cpp
	$(CC) $(CFLAGS) $< -o $@


all:	$(PROG)


clean:
	$(RM) $(PROG) $(OBJS)

res.o : res.rc
	windres -i res.rc -o res.o
