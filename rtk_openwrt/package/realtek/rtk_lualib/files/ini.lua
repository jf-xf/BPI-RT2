local ini = {}

function ini.parse(s)
  local function _parse_l1(s)
    local ans = {}
    local i, j, i2, j2
    j = 0
    while true do
      i2, j2 = s:find("%[[%p%d%a]+%][^%[%]]+%[", j)
      if i2 == nil then break end
      i, j = i2, j2 - 1 -- trailing '['
      --print(i .. " " .. j)
      table.insert(ans, s:sub(i, j))
    end
    table.insert(ans, s:sub(j + 1))

    i = #ans[#ans]
    if ans[#ans]:sub(i, i) ~= "\n" then
      ans[#ans] = ans[#ans] .. "\n"
    end
    return ans
  end
  local function _parse_l2(s)
    local i, j = s:find("%[[%p%d%a]+%]\n")
    local t1 = s:sub(i + 1, j - 2)
    local ens = {}
    i = j + 1
    while true do
      j = s:find("\n", i)
      if j == nil then break end
      table.insert(ens, s:sub(i, j - 1))
      i = j + 1
    end
    return t1, ens
  end
  local function _parse_l3(s)
    local i = s:find("=")
    if not i then return nil end
    local name = s:sub(1, i - 1)
    name = name:gsub("%s+", "")

    local val
    local j = s:find(";")
    if j == nil then
      val = s:sub(i + 1)
    else
      val = s:sub(i + 1, j - 1)
    end
    val = val:gsub("%s+", "")
    return name, val
  end

  local ans = {}
  for i, v in ipairs(_parse_l1(s)) do
    local sect, ens = _parse_l2(v)
    local ens2 = {}
    for j, v2 in ipairs(ens) do
      local ent_name, ent_val = _parse_l3(v2)
      if ent_name then ens2[ent_name] = ent_val end
    end
    ans[sect] = ens2
  end
  return ans
end

function ini.get(ds, ...)
  local arg = { ... }
  local ans = ds
  for i, v in ipairs(arg) do
    ans = ans[v]
  end
  return ans
end

function ini.stringify2(sec, names)
  local ans = ""
  if not names then
    for n, v in pairs(sec) do
      ans = ans .. n .. " = " .. v .. "\n"
    end
  else
    for i, n in ipairs(names) do
      local vv = sec[n] or ""
      ans = ans .. n .. " = " .. vv .. "\n"
    end
  end
  return ans
end

local function strf_l1(secname, sec)
  local ans = ""
  ans = ans .. "[" .. secname .. "]\n"
  ans = ans .. ini.stringify2(sec)
  return ans
end

function ini.stringify(ds)
  local ans = ""
  for secname, sec in pairs(ds) do
    ans = ans .. strf_l1(secname, sec)
  end
  return ans
end

return ini
