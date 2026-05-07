local wpaconf = {}

function wpaconf.parseconf(s)
  local function _parse_l1(s)
    local ans = {}
    local i, j = nil, 0
    local i2, j2
    while true do
      i2, j2 = s:find("[%a_]+={\n", j)
      if i then
        val = s:sub(i, i2 and i2 - 1 or nil)
        val = val:gsub("[ ]+", "")
        val = val:gsub("[ \n]+$", "")
        table.insert(ans, val)
      end
      if not i2 then break end
      i, j = i2, j2
    end
    return ans
  end
  local function _parse_l2(s)
    local i, j = s:find("[%a_]+={\n")
    return s:sub(i, j - 3), s:sub(j + 1, #s - 2)
  end
  local function _parse_l3(s)
    local ans = {}
    for str in s:gmatch("([^\n]+)") do
      local i = str:find("=")
      if i then
        val = str:sub(i + 1)
        if val:match("\".*\"") then val = val:sub(2, #val - 1) end
        ans[str:sub(0, i - 1)] = val
      end
    end
    return ans
  end

  local ans = {}
  for i, v in ipairs(_parse_l1(s)) do
    local sect, ens = _parse_l2(v)
    if not ans[sect] then
      ans[sect] = {}
    end
    table.insert(ans[sect], _parse_l3(ens))
  end
  return ans
end

return wpaconf
