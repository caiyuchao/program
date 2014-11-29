#!/usr/bin/env lua
function read_line(filename)
	local fd = io.open(filename)
	local line = fd:read("*line")
	fd:close()
	return line
end
local line = read_line("/proc/uptime")
local _, _, uptime, idle = string.find(line,'^([0-9.]+)%s+([0-9.]+)$')

print(line, uptime, idle)
