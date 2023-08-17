#!/usr/bin/env lua

local mime = require "mime" -- https://lunarmodules.github.io/luasocket/
local lpeg = require "lpeg" -- https://luarocks.org/modules/gvvaughan/lpeg
-- local vstruct = require "vstruct" -- https://luarocks.org/modules/deepakjois/vstruct

local char, byte = string.char, string.byte
local ldexp = math.ldexp

function fl(cs)
	local b4 = byte(cs:sub(1,1))
	local b3 = byte(cs:sub(2,2))
	local b2 = byte(cs:sub(3,3))
	local b1 = byte(cs:sub(4,4))
	local exponent = (b1 % 128) * 2 + (b2 >> 7)
	local sign = (b1 > 127) and -1 or 1
	local mantissa = ((b2 % 128) * 256 + b3) * 256 + b4
	mantissa = (ldexp(mantissa, -23) + 1) * sign
	return ldexp(mantissa, exponent - 127)
end	

local num = lpeg.C(lpeg.P(4)) / fl   		        -- capture a group of 4 chars and convert it to a float
local extract_floats = lpeg.Ct(num^0)       		-- put the captures into a table

function dec(data,fmt)

    -- Inspired from http://lua-users.org/wiki/BaseSixtyFour

    -- working lua base64 codec (c) 2006-2008 by Alex Kloss
    -- compatible with lua 5.1
    -- http://www.it-rfc.de
    -- licensed under the terms of the LGPL2

    -- decryption table
    -- local base64bytes = {['A']=0,['B']=1,['C']=2,['D']=3,['E']=4,['F']=5,['G']=6,['H']=7,['I']=8,['J']=9,['K']=10,['L']=11,['M']=12,['N']=13,['O']=14,['P']=15,['Q']=16,['R']=17,['S']=18,['T']=19,['U']=20,['V']=21,['W']=22,['X']=23,['Y']=24,['Z']=25,['a']=26,['b']=27,['c']=28,['d']=29,['e']=30,['f']=31,['g']=32,['h']=33,['i']=34,['j']=35,['k']=36,['l']=37,['m']=38,['n']=39,['o']=40,['p']=41,['q']=42,['r']=43,['s']=44,['t']=45,['u']=46,['v']=47,['w']=48,['x']=49,['y']=50,['z']=51,['0']=52,['1']=53,['2']=54,['3']=55,['4']=56,['5']=57,['6']=58,['7']=59,['8']=60,['9']=61,['+']=62,['/']=63,['=']=nil}
	-- local pad = 3 - ((#data-1) % 4)
	-- local result = (data..string.rep('=', pad)):gsub("....", function(cs)
	--   local a = base64bytes[cs:sub(1,1)] or 0
	--   local b = base64bytes[cs:sub(2,2)] or 0
	--   local c = base64bytes[cs:sub(3,3)] or 0
	--   local d = base64bytes[cs:sub(4,4)] or 0
	--   return char((a << 2) + (b >> 4)) .. char(((b << 4) & 255) + (c >> 2)) .. char(((c << 6) & 255) + d)
	-- end)
	local x = os.clock()
	local result = mime.unb64(data)
	local y = os.clock()
	print(string.format("elapsed time for mime.unb64: %.2f\n", y - x))
 
	converted = {}
	if fmt == "f" then
		converted = extract_floats:match(result)   -- match with lpeg pattern

	-- Alternative 1 (slightly slower)
	-- result:gsub("....", function(cs)
	--  local b4 = byte(cs:sub(1,1))
	--  local b3 = byte(cs:sub(2,2))
	--	local b2 = byte(cs:sub(3,3))
	--	local b1 = byte(cs:sub(4,4))
    --  -- Inspired from http://lua-users.org/wiki/ReadWriteFormat
    --  local exponent = (b1 % 128) * 2 + (b2 >> 7)
	--	local sign = (b1 > 127) and -1 or 1
	--	local mantissa = ((b2 % 128) * 256 + b3) * 256 + b4
	--	mantissa = (math.ldexp(mantissa, -23) + 1) * sign 
	-- 	table.insert(converted, math.ldexp(mantissa, exponent - 127))
	-- end)

	-- Alternative 2 (much slower)
	-- format = tostring(#result // 4).."*f4"
	-- readfloats = vstruct.compile(format)
	-- readfloats:read(result, converted)
	elseif fmt == "I" then
		result:gsub("....", function(cs)
			local b4 = byte(cs:sub(1,1))
			local b3 = byte(cs:sub(2,2))
			local b2 = byte(cs:sub(3,3))
			local b1 = byte(cs:sub(4,4))
			table.insert(converted, ((b1*256 + b2)*256 + b3)*256 + b4)
		end)
	elseif fmt == "H" then
		result:gsub("..", function(cs)
			table.insert(converted, byte(cs:sub(2,2)))
			table.insert(converted, byte(cs:sub(1,1)))
		end)
	elseif fmt == "s" then
		converted = result
	else
	  print("not handled (yet)!")
	end

	print(string.format("elapsed time for table creation: %.2f\n", os.clock() - y))
	return converted
end
