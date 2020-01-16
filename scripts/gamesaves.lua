--[[

Functions from C++:

set_mapname(string)
set_mapdims(int, int)   (l, h)
set_maptile(int, int, table)   (x, y, table containing num, owner, burnedfor, usedfor)
set_mappplayers(int)
set_playerdata(int, table)  (player index, table containing name, army number, team number, tint number, controller, cash)
set_playerstats(int, table)  (player index, table containing cash raised, cash spent, units produced, units lost, enemy units killed)
add_playerunit(int, table)  (player index, table containing unit type, tx, ty, health, ability to move, readiness)
set_gameoptions(table)  (table containing {player turn, funds per building, show damage, time limit, time, tech level, turns before oil derricks expire})
add_unit(int, table) (player index, table containing type, tx, ty, health, attackdelay, canmove, ready), RETURNS INDEX OF ADDED UNIT
add_unit_load(int, int, table)  (player index, unit index, table containing type, health), RETURNS INDEX OF ADDED LOAD
add_unit_subload(int, int, int, table)  (player index, unit index, load index, table containing type, health)


Functions to C++:

serialize( string or number )
read_savefile_name(path)
load_savefile(path)

--]]

gamename = ""

function serialize(o)
	if (type(o) == "number") then
		return tostring(o)
	else
		return string.format("%q", o)
	end
end

function read_savefile_name(path)
	savedgame = get_saved_game_name
	dofile(path)
	return gamename
end

function load_savefile(path)
	savedgame = load_saved_game
	dofile(path)
end


--==============


function get_saved_game_name(s)
	gamename = s.name
end

function load_saved_game(s)
	if (type(s) ~= "table") then error("Saved game isn't in a table") end
	
	if not s.name then s.name = "Saved Game" end
	if not s.map then error("Saved game doesn't contain map data") end
	if not s.options then error("Saved game doesn't contain options data") end
	if not s.players then error("Saved game doesn't contain player data") end
	
	set_options(s.options)
	set_map(s.map)
	set_players(s.players)
end


--==============

function set_options(o)
	if (type(o) ~= "table") then error("Game options aren't in a table") end
	
	if ((not goodnum(o.turn)) or (o.turn >= 4)) then error("Saved game's turn field is bad") end
	if not goodnum(o.funding) then error("Saved game's funding option is bad") end
	
	if not o.showdamage then
		o.showdamage = 1
	elseif (o.showdamage ~= 0) then
		o.showdamage = 1
	end
	
	if ((not o.timelimit) or (o.timelimit <= 0)) then o.timelimit = -999 end
	if (o.timelimit > 0) then
		if not o.time then
			o.time = o.timelimit
		elseif o.time > o.timelimit then
			o.time = o.timelimit
		elseif o.time <= 0 then
			o.time = o.timelimit
		end
	else
		o.time = 0
	end
	
	if not goodnum(o.techlevel) then
		o.techlevel = 3
	elseif (o.techlevel > 3) then
		o.techlevel = 3
	end
	
	if not goodnum(o.oilturns) then o.oilturns = 0 end
	
	set_gameoptions(o)
end


--==============


function set_map(m)
	if (type(m) ~= "table") then error("Map data isn't in a table") end
	
	if not goodnum(m.length) then error("Saved game's map doesn't specify map length") end
	if not goodnum(m.height) then error("Saved game's map doesn't specify map height") end
	if not m.tiles then error("Saved game's map doesn't contain tiles") end
	if (type(m.tiles) ~= "table") then error("Saved game's map tiles aren't in a table") end

	set_mapname(m.name)
	set_mapdata(m.length, m.height, m.tiles)
end

function set_mapdata(length, height, tiles)
	numtiles = #tiles
	
	if (numtiles ~= length * height) then
		error("Number of tiles (" .. numtiles.tostring() .. ") doesn't match map length * height")
	end
	
	set_mapdims(length, height)  --set_mapdims clears the map, FYI
	
	x = 0
	y = 0
	for _,n in ipairs(tiles) do
		if not goodnum(n.num) then n.num = 3 end --default to grass tile
		if not goodnum(n.owner) then n.owner = -1 end
		if not goodnum(n.burnedfor) then n.burnedfor = -1 end
		if not goodnum(n.usedfor) then n.usedfor = 0 end
		
		set_maptile(x, y, n)
		
		x = x + 1
		if (x >= length) then
			x = 0
			y = y + 1
		end
	end
end


--==============


function set_players(playertable)
	if (type(playertable) ~= "table") then error("Player data isn't in a table!") end
	
	numplayers = 0
	for _,p in ipairs(playertable) do
		if (type(p) ~= "table") then error("Player entry " .. tostring(numplayers) .. " isn't a table!") end
		
		if p.playing > 0 or p.waseverplaying > 0 then
			if not p.name then p.name = "Player " .. (numplayers + 1) end
			if not p.army then error("Player entry doesn't specify what army it uses") end
			if not p.team then error("Player entry doesn't specify a team") end
			if not p.tintcolor then error("Player entry doesn't specify a tint color") end
			if not p.controller then error("Player entry doesn't specify a controller") end
			if not p.cash then error("Player entry doesn't have a cash entry") end
			
			set_playerdata(numplayers, p)
			
			if ((not p.stats) or (type(p.stats) ~= "table")) then p.stats = {raised=0, spent=0, produced = 0, killed = 0, lost = 0} end
			p.stats.raised   = goodnum(p.stats.raised)   or 0
			p.stats.spent    = goodnum(p.stats.spent)    or 0
			p.stats.produced = goodnum(p.stats.produced) or 0
			p.stats.killed   = goodnum(p.stats.killed)   or 0
			p.stats.lost     = goodnum(p.stats.lost)     or 0
			
			set_playerstats(numplayers, p.stats)
			
			if not p.units then error("Player " .. tostring(numplayers) .. " doesn't have a units field") end
			if (type(p.units) ~= "table") then error("Player " .. tostring(numplayers) .. "'s units field isn't a table") end
			for _,u in ipairs(p.units) do
				set_unit(numplayers, u)
			end
			
			numplayers = numplayers + 1
		end
	end
	
	set_mapplayers(numplayers)
end

function goodnum(s)
	if not s then return nil end
	if (type(s) ~= "number") then return nil end
	if s < 0 then return nil end
	return s
end

function set_unit(player, u)
	if (type(u) ~= "table") then error("Unit field isn't a table") end
	
	if not goodnum(u.type) then error("Bad unit type") end
	if not goodnum(u.x) then error("Bad unit x value") end
	if not goodnum(u.y) then error("Bad unit y value") end
	if ((not goodnum(u.health)) or (u.health > 10)) then error("Bad unit health value") end
	if not u.attackdelay then u.attackdelay = 0 end
	if not goodnum(u.canmove) then u.canmove = 0 end
	if not goodnum(u.ready) then u.ready = 0 end
	
	unit = add_unit(player, u)
	if ((unit < 0) or (unit >= 50)) then error("Error creating unit") end
	
	if ((not u.loadedunits) or (type(u.loadedunits) ~= "table")) then u.loadedunits = {} end
	
	
	for _,l in ipairs(u.loadedunits) do
		if not goodnum(l.type) then error("Bad loadedunits unit type") end
		if ((not goodnum(l.health)) or (l.health > 10)) then error("Bad loadedunits unit health") end
		
		load = add_unit_load(player, unit, l)
		if ((load < 0) or (load >= 2)) then error("Error creating loaded unit") end
		
		if ((not l.subloads) or (type(l.subloads) ~= "table")) then l.subloads = {} end
		
		for _,s in ipairs(l.subloads) do
			if not goodnum(s.type) then error("Bad subload unit type") end
			if ((not goodnum(s.health)) or (s.health > 10)) then error("Bad subload unit health") end
			
			subload = add_unit_subload(player, unit, load, s)
			if ((subload < 0) or (subload >= 2)) then error("Error creating unit subload") end
		end
	end
end


