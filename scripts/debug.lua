﻿

function ram8(label, signed)
  return emu.read(emu.getLabelAddress(label)["address"], emu.memType.nesDebug, signed or false)
end

function ram16(label, signed)
  return emu.readWord(emu.getLabelAddress(label)["address"], emu.memType.nesDebug, signed or false)
end

-- debug perf

scope_count = 4

frame_number = 0


performance_counters = {}
performance_average = {}
for i = 0, scope_count do
  performance_average[i] = {}
end
performance_max = {}
for i = 0, scope_count do
    performance_max[i] = 0
end
last_scope = 0
last_cycle_count = 0

-- Returns the sum of a sequence of values
local function sum(x)
    local s = 0
    for _, v in ipairs(x) do s = s + v end
    return s
end

-- Calculates the arithmetic mean of a set of values
-- x       : an array of values
-- returns : the arithmetic mean
local function arithmetic_mean(x)
    return (sum(x) / #x)
end

color_names = {
    [0]="uncounted",
    [1]="idle",
    [2]="audio",
    [3]="entities",
    [4]="metasprites",
}

function debug_write(address, value)
  local emu_state = emu.getState() 
  local new_scope = value
  local duration = emu_state["cpu.cycleCount"] - last_cycle_count
  last_cycle_count = emu_state["cpu.cycleCount"]
  
  performance_counters[last_scope] = duration
  last_scope = new_scope
end

y_offset = 0

function draw_parameter(key)
  parameter_string = string.format("%s:", color_names[key])
  local value = performance_counters[key]
  if (value ~= nil) then
    if (value >= 0) then
	  emu.drawString(10, y_offset, parameter_string, 0x00FFFFFF, 0x40200020)
	  y_offset = y_offset + 9
	  local mean = arithmetic_mean(performance_average[key])
	  local max = performance_max[key]
	  parameter_string = string.format("  %s (%.1f%%)\t| %s \t| %s", value, value * 100.0 / 29780.5, math.floor(mean), max);
	  emu.drawString(10, y_offset, parameter_string, 0x00FFFFFF, 0x40200020)
	  y_offset = y_offset + 9
    end
  end
end

function draw_performance_counters()
  y_offset = 140
  for i = 0, scope_count do
    local value = performance_counters[i]
    draw_parameter(i)
    performance_average[i][frame_number] = value
    if (value ~= nil and value > performance_max[i]) then
      performance_max[i] = value
    end
    frame_number = frame_number + 1
    if (frame_number > 300) then
      frame_number = 0
    end
  end
end

-- debug collision

OBJECT_COUNT = 12
SOLID_OBJECT_COUNT = 20

OBJECT_OFFSETOF_X_FR = 0 * OBJECT_COUNT
OBJECT_OFFSETOF_X_LO = 1 * OBJECT_COUNT
OBJECT_OFFSETOF_X_HI = 2 * OBJECT_COUNT
OBJECT_OFFSETOF_Y_FR = 3 * OBJECT_COUNT
OBJECT_OFFSETOF_Y_LO = 4 * OBJECT_COUNT
OBJECT_OFFSETOF_Y_HI = 5 * OBJECT_COUNT

OBJECT_OFFSETOF_HITBOX_X = 6 * OBJECT_COUNT
OBJECT_OFFSETOF_HITBOX_Y = 7 * OBJECT_COUNT
OBJECT_OFFSETOF_WIDTH    = 8 * OBJECT_COUNT
OBJECT_OFFSETOF_HEIGHT   = 9 * OBJECT_COUNT

OBJECT_OFFSETOF_STATE = 11 * OBJECT_COUNT

function draw_object_hitbox()
  local objects = emu.getLabelAddress("objects")["address"]

  local room = emu.getLabelAddress("room")["address"]
  local room_x = emu.readWord(room + 3, emu.memType.nesDebug)
  local room_y = emu.readWord(room + 5, emu.memType.nesDebug)
  local scroll_x = emu.read(emu.getLabelAddress("view_x")["address"], emu.memType.nesDebug, false)
  local scroll_y = emu.read(emu.getLabelAddress("view_y")["address"], emu.memType.nesDebug, false)

  -- local scroll_x = ram8("view_x")
  -- local scroll_y = ram8("view_y")
  for i = 0, OBJECT_COUNT-1 do
    local obj = objects + i
    local state = emu.read(obj + OBJECT_OFFSETOF_STATE, emu.memType.nesDebug)
    if (state >= 0x80) then 
      goto continue
    end
    
    local x = emu.read(obj + OBJECT_OFFSETOF_X_LO, emu.memType.nesDebug)
    x = (emu.read(obj + OBJECT_OFFSETOF_X_HI, emu.memType.nesDebug) << 8) | x
    local y = emu.read(obj + OBJECT_OFFSETOF_Y_LO, emu.memType.nesDebug)
    y = (emu.read(obj + OBJECT_OFFSETOF_Y_HI, emu.memType.nesDebug) << 8) | y

    local screen_x = x - room_x - scroll_x
    local screen_y = y - room_y - scroll_y
    -- if (screen_x >= 0 and screen_x <= 256 and screen_y >= 0 and screen_y <= 240) then
      local hb_x = emu.read(obj + OBJECT_OFFSETOF_HITBOX_X, emu.memType.nesDebug)
      local hb_y = emu.read(obj + OBJECT_OFFSETOF_HITBOX_Y, emu.memType.nesDebug)
      local hb_w = emu.read(obj + OBJECT_OFFSETOF_WIDTH, emu.memType.nesDebug)
      local hb_h = emu.read(obj + OBJECT_OFFSETOF_HEIGHT, emu.memType.nesDebug)
      -- draw a rectangle for the obj position
      emu.drawRectangle(screen_x-1,screen_y-1,3,3,0xcc00ff00)
      -- draw a rectangle for the obj hitbox
      emu.drawRectangle(screen_x + hb_x, screen_y + hb_y, hb_w, hb_h, 0x60ff0000)
    -- end
    ::continue::
  end
end

SOLID_OBJECT_OFFSETOF_WIDTH = 5 * SOLID_OBJECT_COUNT
SOLID_OBJECT_OFFSETOF_HEIGHT = 6 * SOLID_OBJECT_COUNT
SOLID_OBJECT_OFFSETOF_X_LO = 1 * SOLID_OBJECT_COUNT
SOLID_OBJECT_OFFSETOF_X_HI = 2 * SOLID_OBJECT_COUNT
SOLID_OBJECT_OFFSETOF_Y_LO = 3 * SOLID_OBJECT_COUNT
SOLID_OBJECT_OFFSETOF_Y_HI = 4 * SOLID_OBJECT_COUNT
SOLID_OBJECT_OFFSETOF_STATE = 0 * SOLID_OBJECT_COUNT

function draw_solid_hitbox()
  local solids = emu.getLabelAddress("solid_objects")["address"]

  local room = emu.getLabelAddress("room")["address"]
  local room_x = emu.readWord(room + 3, emu.memType.nesDebug)
  local room_y = emu.readWord(room + 5, emu.memType.nesDebug)
  local scroll_x = emu.read(emu.getLabelAddress("view_x")["address"], emu.memType.nesDebug, false)
  local scroll_y = emu.read(emu.getLabelAddress("view_y")["address"], emu.memType.nesDebug, false)
  for i = 0, SOLID_OBJECT_COUNT-1 do
    local obj = solids + i
    local state = emu.read(obj + SOLID_OBJECT_OFFSETOF_STATE, emu.memType.nesDebug)
    if (state == 0) then 
      goto continue
    end
    
    local x = emu.read(obj + SOLID_OBJECT_OFFSETOF_X_LO, emu.memType.nesDebug)
    x = (emu.read(obj + SOLID_OBJECT_OFFSETOF_X_HI, emu.memType.nesDebug) << 8) | x
    local y = emu.read(obj + SOLID_OBJECT_OFFSETOF_Y_LO, emu.memType.nesDebug)
    y = (emu.read(obj + SOLID_OBJECT_OFFSETOF_Y_HI, emu.memType.nesDebug) << 8) | y

    local screen_x = x - room_x - scroll_x
    local screen_y = y - room_y - scroll_y
    --emu.log("solid screen_x: "..x)
    --if (screen_x >= 0 and screen_x <= 256 and screen_y >= 0 and screen_y <= 240) then
      --local hb_x = emu.read(obj + SOLID_OBJECT_OFFSETOF_HITBOX_X, emu.memType.nesDebug)
      --local hb_y = emu.read(obj + SOLID_OBJECT_OFFSETOF_HITBOX_Y, emu.memType.nesDebug)
      local hb_w = emu.read(obj + SOLID_OBJECT_OFFSETOF_WIDTH, emu.memType.nesDebug)
      local hb_h = emu.read(obj + SOLID_OBJECT_OFFSETOF_HEIGHT, emu.memType.nesDebug)
      -- draw a rectangle for the obj position
      -- emu.drawRectangle(screen_x-1,screen_y-1,3,3,0xcc00ff00)
      -- draw a rectangle for the obj hitbox
      emu.drawRectangle(screen_x, screen_y, hb_w, hb_h, 0x600000ff)
      emu.drawString(screen_x + hb_w / 2, screen_y + hb_h / 2, ""..i)
    --end
    ::continue::
  end
end

-- debug map

local BOX_WIDTH = 16
local BOX_HEIGHT = 16
local SIZE_OF_ROOM = 8
local SIZE_OF_SECTION = 43
local SECTION_OFFSET = SIZE_OF_ROOM * 32

function load_room(room_id)
  local room_addr = 0x6000 + (room_id)*SIZE_OF_ROOM
  local lead_id = emu.read(room_addr + 0, emu.memType.nesChrRam, false)
  local side_id = emu.read(room_addr + 1, emu.memType.nesChrRam, false)
  local scroll = emu.read(room_addr + 2, emu.memType.nesChrRam, false)
  local x = emu.read(room_addr + 3, emu.memType.nesChrRam, false)
  local y = emu.read(room_addr + 5, emu.memType.nesChrRam, false)
  local prize = emu.read(room_addr + 7, emu.memType.nesChrRam, false)
  return lead_id, side_id, scroll, x, y, prize
end

function draw_section(section_id, other_id, is_lead)
  local section_addr = 0x6000 + SECTION_OFFSET + (section_id * SIZE_OF_SECTION)
  local other_addr = 0x6000 + SECTION_OFFSET + (other_id * SIZE_OF_SECTION)
  local room_id = emu.read(section_addr + 0, emu.memType.nesChrRam, false)
  local nametable = emu.read(section_addr + 1, emu.memType.nesChrRam, false)
  local room_base = emu.read(section_addr + 2, emu.memType.nesChrRam, false)
  local exits = {}
  for i=1, 4 do
    exits[i] = emu.read(section_addr + 3 + i - 1, emu.memType.nesChrRam, false)
  end
  -- emu.log(string.format("section_id: %d exits: %02x, %02x, %02x, %02x", section_id, exits[1], exits[2], exits[3], exits[4]))
  local objects = {}
  -- for i=1, 6 do
  --   objects[i] = emu.read(section_addr + 7 + i - 1, emu.memType.nesChrRam, false)
  -- end
  -- return room_id, nametable, room_base, exits --, objects
  local grid_x = section_id // 8
  local grid_y = section_id % 8
  local box_x = 256 - (8-grid_x)*BOX_WIDTH
  local box_y = (grid_y)*BOX_HEIGHT
  if (is_lead == true) then
    emu.drawString(box_x+6, box_y+4, "L")
  else
    emu.drawString(box_x+6, box_y+4, "S")
  end
  -- emu.drawString(box_x, box_y, string.format("%d%d", grid_x, grid_y))
  -- emu.log(string.format("section_id: %02x exits: %02x, %02x, %02x, %02x", section_id, exits[1], exits[2], exits[3], exits[4]))
  for i=1, 4 do
    local ex = exits[i]
    -- emu.log(ex)
    if (ex == 0xe0) then
      -- draw a small green box to mark that this is a neighbor room
      local pair_color = 0x00ff00
      if (i == 1) then
        emu.drawRectangle(box_x + BOX_WIDTH/2 - 2, box_y - 2, 4, 4, pair_color, true)
      elseif (i == 2) then
        emu.drawRectangle(box_x + BOX_WIDTH-2, box_y + BOX_HEIGHT/2 - 2, 4, 4, pair_color, true)
      elseif (i == 3) then
        emu.drawRectangle(box_x + BOX_WIDTH/2 - 2, box_y + BOX_HEIGHT-2, 4, 4, pair_color, true)
      else
        emu.drawRectangle(box_x - 2, box_y + BOX_HEIGHT/2 - 2, 4, 4, pair_color, true)
      end
    elseif (ex < 0xe0) then
      -- draw a unique color to mark the exits
      local pair_color = 0x0000ff
      if (is_lead) then
        pair_color = 0xff0000
      end
      if (i == 1) then
        emu.drawRectangle(box_x+2, box_y, BOX_WIDTH-4, 2, 0xff0000, true)
      elseif (i == 2) then
        emu.drawRectangle(box_x+BOX_WIDTH-2, box_y+2, 2, BOX_HEIGHT-4, 0xffff00, true)
      elseif (i == 3) then
        emu.drawRectangle(box_x+2, box_y+BOX_HEIGHT-2, BOX_WIDTH-4, 2, 0x0000ff, true)
      else
        emu.drawRectangle(box_x, box_y+2, 2, BOX_HEIGHT-4, 0xff00ff, true)
      end
    else
      -- draw a white bar to mark the walls
      if (i == 1) then
        emu.drawRectangle(box_x, box_y, BOX_WIDTH, 2, 0xffffff, true)
      elseif (i == 2) then
        emu.drawRectangle(box_x+BOX_WIDTH, box_y, 2, BOX_HEIGHT, 0xffffff, true)
      elseif (i == 3) then
        emu.drawRectangle(box_x, box_y+BOX_HEIGHT, BOX_WIDTH, 2, 0xffffff, true)
      else
        emu.drawRectangle(box_x, box_y, 2, BOX_HEIGHT, 0xffffff, true)
      end
    end
  end
end

function dump_map()
  -- emu.log("Dumping map")
  local objects = emu.getLabelAddress("objects")["address"]
  local player_x = emu.read(objects + OBJECT_OFFSETOF_X_HI, emu.memType.nesDebug)
  -- x = (emu.read(objects + OBJECT_OFFSETOF_X_HI, emu.memType.nesDebug) << 8) | x
  local player_y = emu.read(objects + OBJECT_OFFSETOF_Y_HI, emu.memType.nesDebug)
  -- y = (emu.read(objects + OBJECT_OFFSETOF_Y_HI, emu.memType.nesDebug) << 8) | y
  for i = 0, 8 do
    for j = 0, 8 do
      local box_x = 256 - (8-i)*BOX_WIDTH
      local box_y = (j)*BOX_HEIGHT
      emu.drawRectangle(box_x, box_y, BOX_WIDTH, BOX_HEIGHT, 0xaa777777)
      local room_id = i*8 + (j)
      lead_id, side_id, scroll, x, y, prize = load_room(room_id)
      if (lead_id == 0xff and side_id == 0xff) then
        goto continue
      end
      -- emu.log(string.format("room_id: %d, lead_id %02x, side_id %02x, scroll %02x, x %02x, y %02x, prize: %02x", room_id, lead_id, side_id, scroll, x, y, prize))

      -- we have an actual room so load the lead and side section data
      draw_section(lead_id, side_id, true)
      draw_section(side_id, lead_id, false)

      -- local room = emu.getLabelAddress("room")["address"]
      -- local room_x = emu.readWord(room + 3, emu.memType.nesDebug)
      -- local room_y = emu.readWord(room + 5, emu.memType.nesDebug)
      if (player_x == i and player_y == j) then
        emu.log(string.format("x: %02x, y: %02x", player_x, player_y))
        -- local grid_x = room_id // 8
        -- local grid_y = room_id % 8
        local _x = 256 - (8-i)*BOX_WIDTH
        local _y = (j)*BOX_HEIGHT
        emu.drawString(_x+6, _y+4, "X")
      end
      ::continue::
    end
  end
end

function frame_start()
  draw_object_hitbox()
  draw_solid_hitbox()
  dump_map()
  -- draw_performance_counters()
  --now reset performance counters for the next frame
  for i = 0, scope_count do
    performance_counters[i] = 0
  end
end

emu.addEventCallback(frame_start, emu.eventType.nmi)


str = ""
function cb(address, value)
  c = string.char(value)
  if (c == '\n') then
    emu.log(str)
    str = ""
  end
  str = str .. c
  if (string.len(str) > 80) then
    emu.log(str)
    str = ""
  end
end

emu.addMemoryCallback(cb, emu.callbackType.write, 0x401b)
emu.addMemoryCallback(debug_write, emu.callbackType.write, 0x4123)
