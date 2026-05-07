--[[
 *
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 *
--]]

local logger = { outfile = "ca.log" }

local _tostring = tostring

local tostring = function(...)
	local t = {}
	for i = 1, select('#', ...) do
		local x = select(i, ...)
		t[#t + 1] = _tostring(x)
	end
	return table.concat(t, " ")
end

function logger.init(file_path)
	logger.outfile = file_path
end

function logger.log(...)
	local msg = tostring(...)
	local string = string.format("CA [%s]: %s\n", os.date("%H:%M:%S"), msg)
	print(string)
	local fp = io.open(logger.outfile, "a")
	fp:write(string)
	fp:close()
end

function logger.debug(...)
	local msg = tostring(...)
	local info = debug.getinfo(2, "Sl")
	local lineinfo = info.short_src .. ":" .. info.currentline
	local string = string.format("CA [%s] %s: %s", os.date("%H:%M:%S"), lineinfo, msg)
	print(string)
	local fp = io.open(logger.outfile, "a")
	fp:write(str)
	fp:close()
end

return logger