local X = {}

function X.trim1(s)
  return (s:gsub("^%s*(.-)%s*$", "%1"))
end

function X.ssplit(s, sep)
  if s == "" then return {} end
  sep = sep or ","
  local t = {}
  local i = 1
  while true do
    local ii = s:find(sep, i)
    if not ii then
      table.insert(t, s:sub(i))
      break
    end
    table.insert(t, s:sub(i, ii - 1))
    i = ii + 1
  end
  return t
end

function X.sjoin(tkns, sep)
  if #tkns == 0 then
    return ""
  end
  sep = sep or ","
  local ans = ""
  for i, v in ipairs(tkns) do
    ans = ans .. v .. sep
  end
  return ans:sub(1, #ans - 1)
end

return X
