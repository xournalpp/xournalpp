#!/usr/bin/env lua

-- From http://lua-users.org/wiki/BaseSixtyFour

-- working lua base64 codec (c) 2006-2008 by Alex Kloss
-- compatible with lua 5.1
-- http://www.it-rfc.de
-- licensed under the terms of the LGPL2

-- with modifications by Roland Lötscher in 2022


-- bitshift functions (<<, >> equivalent)
-- shift left
function lsh(value,shift)
	return (value*(2^shift)) % 256
end

-- shift right
function rsh(value,shift)
	return math.floor(value/2^shift) % 256
end

-- return single bit (for OR)
function bit(x,b)
	return (x % 2^b - x % 2^(b-1) > 0)
end

-- logic OR for number values
function lor(x,y)
	result = 0
	for p=1,8 do result = result + (((bit(x,p) or bit(y,p)) == true) and 2^(p-1) or 0) end
	return result
end

-- decryption table
local base64bytes = {['A']=0,['B']=1,['C']=2,['D']=3,['E']=4,['F']=5,['G']=6,['H']=7,['I']=8,['J']=9,['K']=10,['L']=11,['M']=12,['N']=13,['O']=14,['P']=15,['Q']=16,['R']=17,['S']=18,['T']=19,['U']=20,['V']=21,['W']=22,['X']=23,['Y']=24,['Z']=25,['a']=26,['b']=27,['c']=28,['d']=29,['e']=30,['f']=31,['g']=32,['h']=33,['i']=34,['j']=35,['k']=36,['l']=37,['m']=38,['n']=39,['o']=40,['p']=41,['q']=42,['r']=43,['s']=44,['t']=45,['u']=46,['v']=47,['w']=48,['x']=49,['y']=50,['z']=51,['0']=52,['1']=53,['2']=54,['3']=55,['4']=56,['5']=57,['6']=58,['7']=59,['8']=60,['9']=61,['+']=62,['/']=63,['=']=nil}

-- function decode
-- decode base64 input to various formats

function dec(data,fmt)
	local chars = {}
	local result = {}
	for dpos=0,string.len(data)-1,4 do
		for char=1,4 do 
		  chars[char] = base64bytes[(string.sub(data,(dpos+char),(dpos+char)) or "=")]
		  if chars[char] == nil then chars[char] = 0; print("Invalid char, dpos = " .. dpos .. "char = " .. char) end
		end
		table.insert(result, lor(lsh(chars[1],2), rsh(chars[2],4)))
		table.insert(result, (chars[3] ~= nil) and lor(lsh(chars[2],4), rsh(chars[3],2)) or 0)
		table.insert(result, (chars[4] ~= nil) and lor(lsh(chars[3],6), chars[4]) or 0)
	end
	converted = {}
	for i=1, #result//4 do
	  chr = {}
	  for j=1,4 do
	    chr[j]=result[4*(i-1)+j]
	  end
	  if fmt == "f" then
	  	table.insert(converted, convertfloat(chr))
	  elseif fmt == "u" then
		table.insert(converted, convertuint(chr))
	  elseif fmt == "s" then
		table.insert(converted, convertstring(chr))
	  elseif fmt == "b" then
		for m = 1, 4 do table.insert(converted, chr[m]) end
	  else
		print("not handled (yet)!")
	  end
	end
	return converted, result
end

-- From http://lua-users.org/wiki/ReadWriteFormat
function convertfloat(chr)
  local b4,b3,b2,b1 = table.unpack(chr)
  local exponent = (b1 % 128) * 2 + math.floor(b2 / 128)
  if exponent == 0 then return 0 end
  local sign = (b1 > 127) and -1 or 1
  local mantissa = ((b2 % 128) * 256 + b3) * 256 + b4
  mantissa = (math.ldexp(mantissa, -23) + 1) * sign
  return math.ldexp(mantissa, exponent - 127)
end

-- Simple conversion to uint
function convertuint(chr)
	local b4,b3,b2,b1 = table.unpack(chr)
	return ((b1*256 + b2)*256 + b3)*256 + b4
end

-- Simple conversion to string
function convertstring(chr)
	local b4,b3,b2,b1 = table.unpack(chr)
	return string.char(b4) + string.char(b3) + string.char(b2) + string.char(b1)
end

function convertubyte(chr)
	local b4,b3,b2,b1 = table.unpack(chr)
	return {b4, b2, b2, b1}
end
