

scope_count = 2

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
    -- [3]="entities",
    -- [4]="metasprites",
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
  emu.drawString(10, y_offset, parameter_string, 0x00FFFFFF, 0x40200020)
  y_offset = y_offset + 9
  local value = performance_counters[key]
  local mean = arithmetic_mean(performance_average[key])
  local max = performance_max[key]
  parameter_string = string.format("  %s \t| %s \t| %s", value, math.floor(mean), max);
  emu.drawString(10, y_offset, parameter_string, 0x00FFFFFF, 0x40200020)
  y_offset = y_offset + 9
end

function draw_performance_counters()
  y_offset = 10
  for i = 0, scope_count do
    local value = performance_counters[i]
    draw_parameter(i)
    performance_average[i][frame_number] = value
    if (value > performance_max[i]) then
      performance_max[i] = value
    end
    frame_number = frame_number + 1
    if (frame_number > 300) then
      frame_number = 0
    end
  end
end

function frame_start()
  draw_performance_counters()
  --now reset performance counters for the next frame
  for i = 0, scope_count do
    performance_counters[i] = 0
  end
end


emu.addEventCallback(frame_start, emu.eventType.nmi)
emu.addMemoryCallback(debug_write, emu.callbackType.write, 0x4123)