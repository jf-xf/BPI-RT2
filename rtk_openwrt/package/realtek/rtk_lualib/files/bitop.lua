local X = {
  OR = 1,
  XOR = 3,
  AND = 4
}

function X.bitoper(a, b, oper)
  local r, m, s = 0, 2^31
  repeat
     s,a,b = a+b+m, a%m, b%m
     r,m = r + m*oper%(s-a-b), m/2
  until m < 1
  return r
end

function X._or(a, b)
  return X.bitoper(a, b, X.OR)
end

function X._and(a, b)
  return X.bitoper(a, b, X.AND)
end

function X._xor(a, b)
  return X.bitoper(a, b, X.XOR)
end

function X._not(n)
  local p,c=1,0
  while n>0 do
    local r=n%2
    if r<1 then c=c+p end
    n,p=(n-r)/2,p*2
  end
  return c
end

function X._isMaskValid(mask)
  -- check if mask has only one bit set
  return mask ~= 0 and (X._and(mask, mask-1)) == 0
end

function X._isBitSet(n, mask)
  if not X._isMaskValid(mask) then return end
  local result = X._and(n, mask)
  if result == mask then
    return true
  else
    return false
  end
end

function X._clearBit(n, mask)
  if not X._isMaskValid(mask) then return end
  return X._and(n, X._not(mask))
end

function X._setBit(n, mask)
  if not X._isMaskValid(mask) then return end
  return X._or(n, mask)
end

return X

--print(bitoper(6,3,OR))   --> 7
--print(bitoper(6,3,XOR))  --> 5
--print(bitoper(6,3,AND))  --> 2
