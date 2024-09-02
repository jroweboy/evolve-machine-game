

function ram8(label, signed)
  return emu.read(emu.getLabelAddress(label)["address"], emu.memType.nesDebug, signed or false)
end

function ram16(label, signed)
  return emu.readWord(emu.getLabelAddress(label)["address"], emu.memType.nesDebug, signed or false)
end


local BOX_WIDTH = 16
local BOX_HEIGHT = 16
local SIZE_OF_ROOM = 6
local SIZE_OF_SECTION = 43
local SECTION_OFFSET = SIZE_OF_ROOM * 32

function draw_section(section_id, other_id, is_lead)
  local section_addr = SECTION_OFFSET + (section_id * SIZE_OF_SECTION)
  local other_addr = SECTION_OFFSET + (other_id * SIZE_OF_SECTION)
-- emu.log("section_id: "..section_id.." other_id: "..other_id)
  local room_id = emu.read(section_addr + 0, emu.memType.nesChrRam, false)
  local nametable = emu.read(section_addr + 1, emu.memType.nesChrRam, false)
  local room_base = emu.read(section_addr + 2, emu.memType.nesChrRam, false)
  local exits = {}
  for i=1, 4 do
    exits[i] = emu.read(section_addr + 3 + i - 1, emu.memType.nesChrRam, false)
  end
  local objects = {}
  -- for i=1, 6 do
  --   objects[i] = emu.read(section_addr + 7 + i - 1, emu.memType.nesChrRam, false)
  -- end
  -- return room_id, nametable, room_base, exits --, objects
  local grid_x = section_id // 8
  local grid_y = section_id % 8
  local box_x = 256 - (8-grid_x)*BOX_WIDTH
  local box_y = (8-grid_y)*BOX_HEIGHT
  if (is_lead == true) then
    emu.drawString(box_x+6, box_y+4, "L")
  else
    emu.drawString(box_x+6, box_y+4, "S")
  end

  for i=1, 4 do
    local ex = exits[i]
    -- emu.log(ex)
    if (ex < 0xe0) then
      local pair_color = 0xff0000ff
      if (is_lead) then
        pair_color = 0xffff0000
      end
      if (i == 1) then
        emu.drawRectangle(box_x+2, box_y, BOX_WIDTH-4, 2, pair_colo, true)
      elseif (i == 2) then
        emu.drawRectangle(box_x+BOX_WIDTH-2, box_y+2, 2, BOX_HEIGHT-4, pair_color, true)
      elseif (i == 3) then
        emu.drawRectangle(box_x+2, box_y+BOX_HEIGHT-2, BOX_WIDTH-4, 2, pair_color, true)
      else
        emu.drawRectangle(box_x, box_y+2, 2, BOX_HEIGHT-4, pair_color, true)
      end
    end
  end
end

function dump_map()
  for i = 0, 8 do
    for j = 0, 8 do
      local box_x = 256 - (8-i)*BOX_WIDTH
      local box_y = (8-j)*BOX_HEIGHT
      emu.drawRectangle(box_x, box_y, BOX_WIDTH, BOX_HEIGHT, 0xaa777777)
      local room_addr = 0x6000 + (i*8+j)*8
      local room_id = i*8 + (j)
      local lead_id = emu.read(room_addr + 0, emu.memType.nesChrRam, false)
      local side_id = emu.read(room_addr + 1, emu.memType.nesChrRam, false)
      local scroll = emu.read(room_addr + 2, emu.memType.nesChrRam, false)
      local x = emu.read(room_addr + 3, emu.memType.nesChrRam, false)
      local y = emu.read(room_addr + 4, emu.memType.nesChrRam, false)
      local prize = emu.read(room_addr + 5, emu.memType.nesChrRam, false)
      if (lead_id ~= 0xff or side_id ~= 0xff) then
        -- we have an actual room so load the lead and side section data
        draw_section(lead_id, side_id, true)
        draw_section(side_id, lead_id, false)
      end 
    end
  end
end


-- OBJECT_COUNT = 12
-- SOLID_OBJECT_COUNT = 20

-- OBJECT_OFFSETOF_HITBOX_X = 4 * OBJECT_COUNT
-- OBJECT_OFFSETOF_HITBOX_Y = 5 * OBJECT_COUNT
-- OBJECT_OFFSETOF_WIDTH = 6 * OBJECT_COUNT
-- OBJECT_OFFSETOF_HEIGHT = 7 * OBJECT_COUNT
-- OBJECT_OFFSETOF_X_LO = 0 * OBJECT_COUNT
-- OBJECT_OFFSETOF_X_HI = 1 * OBJECT_COUNT
-- OBJECT_OFFSETOF_Y_LO = 2 * OBJECT_COUNT
-- OBJECT_OFFSETOF_Y_HI = 3 * OBJECT_COUNT
-- OBJECT_OFFSETOF_STATE = 9 * OBJECT_COUNT

-- function draw_object_hitbox()
--   local objects = emu.getLabelAddress("objects")["address"]

--   local room = emu.getLabelAddress("room")["address"]
--   local room_x = emu.readWord(room + 3, emu.memType.nesDebug)
--   local room_y = emu.readWord(room + 5, emu.memType.nesDebug)
--   local scroll_x = ram8("view_x")
--   local scroll_y = ram8("view_y")  
--   for i = 0, OBJECT_COUNT-1 do
--     local obj = objects + i
--     local state = emu.read(obj + OBJECT_OFFSETOF_STATE, emu.memType.nesDebug)
--     if (state >= 0x80) then 
--       goto continue
--     end
    
--     local x = emu.read(obj + OBJECT_OFFSETOF_X_LO, emu.memType.nesDebug)
--     x = (emu.read(obj + OBJECT_OFFSETOF_X_HI, emu.memType.nesDebug) << 8) | x
--     local y = emu.read(obj + OBJECT_OFFSETOF_Y_LO, emu.memType.nesDebug)
--     y = (emu.read(obj + OBJECT_OFFSETOF_Y_HI, emu.memType.nesDebug) << 8) | y

--     local screen_x = x - room_x - scroll_x
--     local screen_y = y - room_y - scroll_y
--     -- if (screen_x >= 0 and screen_x <= 256 and screen_y >= 0 and screen_y <= 240) then
--       local hb_x = emu.read(obj + OBJECT_OFFSETOF_HITBOX_X, emu.memType.nesDebug)
--       local hb_y = emu.read(obj + OBJECT_OFFSETOF_HITBOX_Y, emu.memType.nesDebug)
--       local hb_w = emu.read(obj + OBJECT_OFFSETOF_WIDTH, emu.memType.nesDebug)
--       local hb_h = emu.read(obj + OBJECT_OFFSETOF_HEIGHT, emu.memType.nesDebug)
--       -- draw a rectangle for the obj position
--       emu.drawRectangle(screen_x-1,screen_y-1,3,3,0xcc00ff00)
--       -- draw a rectangle for the obj hitbox
--       emu.drawRectangle(screen_x + hb_x, screen_y + hb_y, hb_w, hb_h, 0x60ff0000)
--     -- end
--     ::continue::
--   end
-- end

-- SOLID_OBJECT_OFFSETOF_WIDTH = 5 * SOLID_OBJECT_COUNT
-- SOLID_OBJECT_OFFSETOF_HEIGHT = 6 * SOLID_OBJECT_COUNT
-- SOLID_OBJECT_OFFSETOF_X_LO = 1 * SOLID_OBJECT_COUNT
-- SOLID_OBJECT_OFFSETOF_X_HI = 2 * SOLID_OBJECT_COUNT
-- SOLID_OBJECT_OFFSETOF_Y_LO = 3 * SOLID_OBJECT_COUNT
-- SOLID_OBJECT_OFFSETOF_Y_HI = 4 * SOLID_OBJECT_COUNT
-- SOLID_OBJECT_OFFSETOF_STATE = 0 * SOLID_OBJECT_COUNT

-- function draw_solid_hitbox()
--   local solids = emu.getLabelAddress("solid_objects")["address"]

--   local room = emu.getLabelAddress("room")["address"]
--   local room_x = emu.readWord(room + 3, emu.memType.nesDebug)
--   local room_y = emu.readWord(room + 5, emu.memType.nesDebug)
--   local scroll_x = ram8("view_x")
--   local scroll_y = ram8("view_y")  
--   for i = 0, SOLID_OBJECT_COUNT-1 do
--     local obj = solids + i
--     local state = emu.read(obj + SOLID_OBJECT_OFFSETOF_STATE, emu.memType.nesDebug)
--     if (state == 0) then 
--       goto continue
--     end
    
--     local x = emu.read(obj + SOLID_OBJECT_OFFSETOF_X_LO, emu.memType.nesDebug)
--     x = (emu.read(obj + SOLID_OBJECT_OFFSETOF_X_HI, emu.memType.nesDebug) << 8) | x
--     local y = emu.read(obj + SOLID_OBJECT_OFFSETOF_Y_LO, emu.memType.nesDebug)
--     y = (emu.read(obj + SOLID_OBJECT_OFFSETOF_Y_HI, emu.memType.nesDebug) << 8) | y

--     local screen_x = x - room_x - scroll_x
--     local screen_y = y - room_y - scroll_y
--     --emu.log("solid screen_x: "..x)
--     --if (screen_x >= 0 and screen_x <= 256 and screen_y >= 0 and screen_y <= 240) then
--       --local hb_x = emu.read(obj + SOLID_OBJECT_OFFSETOF_HITBOX_X, emu.memType.nesDebug)
--       --local hb_y = emu.read(obj + SOLID_OBJECT_OFFSETOF_HITBOX_Y, emu.memType.nesDebug)
--       local hb_w = emu.read(obj + SOLID_OBJECT_OFFSETOF_WIDTH, emu.memType.nesDebug)
--       local hb_h = emu.read(obj + SOLID_OBJECT_OFFSETOF_HEIGHT, emu.memType.nesDebug)
--       -- draw a rectangle for the obj position
--       -- emu.drawRectangle(screen_x-1,screen_y-1,3,3,0xcc00ff00)
--       -- draw a rectangle for the obj hitbox
--       emu.drawRectangle(screen_x, screen_y, hb_w, hb_h, 0x600000ff)
--       emu.drawString(screen_x + hb_w / 2, screen_y + hb_h / 2, ""..i)
--     --end
--     ::continue::
--   end
-- end


-- local map = emu.getLabelAddress("map")["address"]


function frame_start()
  -- local solid_objects = emu.getLabelAddress("solid_objects")["address"]
  -- draw_object_hitbox()
  -- draw_solid_hitbox()
  dump_map()
end

emu.addEventCallback(frame_start, emu.eventType.nmi)


-- str = ""
-- function cb(address, value)
--   c = string.char(value)
--   if (c == '\n') then
--     emu.log(str)
--     str = ""
--   end
--   str = str .. c
--   if (string.len(str) > 80) then
--     emu.log(str)
--     str = ""
--   end
-- end

-- emu.addMemoryCallback(cb, emu.callbackType.write, 0x401b)
