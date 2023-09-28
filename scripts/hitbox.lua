

function ram8(label, signed)
  return emu.read(emu.getLabelAddress(label)["address"], emu.memType.nesDebug, signed or false)
end

function ram16(label, signed)
  return emu.readWord(emu.getLabelAddress(label)["address"], emu.memType.nesDebug, signed or false)
end

OBJECT_COUNT = 12
SOLID_OBJECT_COUNT = 20

OBJECT_OFFSETOF_HITBOX_X = 4 * OBJECT_COUNT
OBJECT_OFFSETOF_HITBOX_Y = 5 * OBJECT_COUNT
OBJECT_OFFSETOF_WIDTH = 6 * OBJECT_COUNT
OBJECT_OFFSETOF_HEIGHT = 7 * OBJECT_COUNT
OBJECT_OFFSETOF_X_LO = 0 * OBJECT_COUNT
OBJECT_OFFSETOF_X_HI = 1 * OBJECT_COUNT
OBJECT_OFFSETOF_Y_LO = 2 * OBJECT_COUNT
OBJECT_OFFSETOF_Y_HI = 3 * OBJECT_COUNT
OBJECT_OFFSETOF_STATE = 9 * OBJECT_COUNT

function draw_object_hitbox()
  local objects = emu.getLabelAddress("objects")["address"]

  local room = emu.getLabelAddress("room")["address"]
  local room_x = emu.readWord(room + 3, emu.memType.nesDebug)
  local room_y = emu.readWord(room + 5, emu.memType.nesDebug)
  local scroll_x = ram8("view_x")
  local scroll_y = ram8("view_y")  
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
  local scroll_x = ram8("view_x")
  local scroll_y = ram8("view_y")  
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

function frame_start()
  local solid_objects = emu.getLabelAddress("solid_objects")["address"]
  draw_object_hitbox()
  draw_solid_hitbox()
end

emu.addEventCallback(frame_start, emu.eventType.nmi)



