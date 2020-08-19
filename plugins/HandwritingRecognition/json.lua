-----------------------------------------------------------------------------
-- JSON4Lua: JSON encoding / decoding / traversing support for the Lua language.
-- json Module.
-- Authors: Craig Mason-Jones, Egor Skriptunoff

-- Version: 1.2.1
-- 2017-05-10

-- This module is released under the MIT License (MIT).
-- Please see LICENCE.txt for details:
-- https://github.com/craigmj/json4lua/blob/master/doc/LICENCE.txt
--
-- USAGE:
-- This module exposes three functions:
--   json.encode(obj)
--     Accepts Lua value (table/string/boolean/number/nil/json.null/json.empty) and returns JSON string.
--   json.decode(s)
--     Accepts JSON (as string or as loader function) and returns Lua object.
--   json.traverse(s, callback)
--     Accepts JSON (as string or as loader function) and user-supplied callback function, returns nothing
--     Traverses the JSON, sends each item to callback function, no memory-consuming Lua objects are being created.

--
-- REQUIREMENTS:
--   Lua 5.1, Lua 5.2, Lua 5.3 or LuaJIT
--
-- CHANGELOG
--   1.2.1   Now you can partially decode JSON while traversing it (callback function should return true).
--   1.2.0   Some improvements made to be able to use this module on RAM restricted devices:
--             To read large JSONs, you can now provide "loader function" instead of preloading whole JSON as Lua string.
--             Added json.traverse() to traverse JSON using callback function (without creating arrays/objects in Lua).
--             Now, instead of decoding whole JSON, you can decode its arbitrary element (e.g, array or object)
--                by specifying the position where this element starts.
--                In order to do that, at first you have to traverse JSON to get all positions you need.
--           Most of the code rewritten to improve performance.
--           Decoder now understands extended syntax beyond strict JSON standard:
--             In arrays:
--                trailing comma is ignored:            [1,2,3,]     -> [1,2,3]
--                missing values are nulls:             [,,42,,]     -> [null,null,42,null]
--             In objects:
--                missing values are ignored:           {,,"a":42,,} -> {"a":42}
--                unquoted identifiers are valid keys:  {a$b_5:42}   -> {"a$b_5":42}
--           Encoder now accepts both 0-based and 1-based Lua arrays (but decoder always converts JSON arrays to 1-based Lua arrays).
--           Some minor bugs fixed.
--   1.1.0   Modifications made by Egor Skriptunoff, based on version 1.0.0 taken from
--              https://github.com/craigmj/json4lua/blob/40fb13b0ec4a70e36f88812848511c5867bed857/json/json.lua.
--           Added Lua 5.2 and Lua 5.3 compatibility.
--           Removed Lua 5.0 compatibility.
--           Introduced json.empty (Lua counterpart for empty JSON object)
--           Bugs fixed:
--              Attempt to encode Lua table {[10^9]=0} raises an out-of-memory error.
--              Zero bytes '\0' in Lua strings are not escaped by encoder.
--              JSON numbers with capital "E" (as in 1E+100) are not accepted by decoder.
--              All nulls in a JSON arrays are skipped by decoder, sparse arrays could not be loaded correctly.
--              UTF-16 surrogate pairs in JSON strings are not recognised by decoder.
--   1.0.0   Merged Amr Hassan's changes
--   0.9.30  Changed to MIT Licence.
--   0.9.20  Introduction of local Lua functions for private functions (removed _ function prefix).
--           Fixed Lua 5.1 compatibility issues.
--           Introduced json.null to have null values in associative arrays.
--           json.encode() performance improvement (more than 50%) through table_concat rather than ..
--           Introduced decode ability to ignore /**/ comments in the JSON string.
--   0.9.10  Fix to array encoding / decoding to correctly manage nil/null values in arrays.
--   0.9.00  First release
--
-----------------------------------------------------------------------------

-----------------------------------------------------------------------------
-- Module declaration
-----------------------------------------------------------------------------
local json = {}

do
   -----------------------------------------------------------------------------
   -- Imports and dependencies
   -----------------------------------------------------------------------------
   local math, string, table = require'math', require'string', require'table'
   local math_floor, math_max, math_type = math.floor, math.max, math.type or function() end
   local string_char, string_sub, string_find, string_match, string_gsub, string_format
      = string.char, string.sub, string.find, string.match, string.gsub, string.format
   local table_insert, table_remove, table_concat = table.insert, table.remove, table.concat
   local type, tostring, pairs, assert, error = type, tostring, pairs, assert, error
   local loadstring = loadstring or load

   -----------------------------------------------------------------------------
   -- Public functions
   -----------------------------------------------------------------------------
   -- function  json.encode(obj)       encodes Lua value to JSON, returns JSON as string.
   -- function  json.decode(s, pos)    decodes JSON, returns the decoded result as Lua value (may be very memory-consuming).

   --    Both functions json.encode() and json.decode() work with "special" Lua values json.null and json.empty
   --       special Lua value  json.null    =  JSON value  null
   --       special Lua value  json.empty   =  JSON value  {}     (empty JSON object)
   --       regular Lua empty table         =  JSON value  []     (empty JSON array)

   --    Empty JSON objects and JSON nulls require special handling upon sending (encoding).
   --       Please make sure that you send empty JSON objects as json.empty (instead of empty Lua table).
   --       Empty Lua tables will be encoded as empty JSON arrays, not as empty JSON objects!
   --          json.encode( {empt_obj = json.empty, empt_arr = {}} )   -->   {"empt_obj":{},"empt_arr":[]}
   --       Also make sure you send JSON nulls as json.null (instead of nil).
   --          json.encode( {correct = json.null, incorrect = nil} )   -->   {"correct":null}

   --    Empty JSON objects and JSON nulls require special handling upon receiving (decoding).
   --       After receiving the result of decoding, every Lua table returned (including nested tables) should firstly
   --       be compared with special Lua values json.empty/json.null prior to making operations on these values.
   --       If you don't need to distinguish between empty JSON objects and empty JSON arrays,
   --       json.empty may be replaced with newly created regular empty Lua table.
   --          v = (v == json.empty) and {} or v
   --       If you don't need special handling of JSON nulls, you may replace json.null with nil to make them disappear.
   --          if v == json.null then v = nil end

   -- Function  json.traverse(s, callback, pos)  traverses JSON using user-supplied callback function, returns nothing.
   --    Traverse is useful to reduce memory usage: no memory-consuming objects are being created in Lua while traversing.
   --    Each item found inside JSON will be sent to callback function passing the following arguments:
   --    (path, json_type, value, pos, pos_last)
   --       path      is array of nested JSON identifiers/indices, "path" is empty for root JSON element
   --       json_type is one of "null"/"boolean"/"number"/"string"/"array"/"object"
   --       value     is defined when json_type is "null"/"boolean"/"number"/"string", value == nil for "object"/"array"
   --       pos       is 1-based index of first character of current JSON element
   --       pos_last  is 1-based index of last character of current JSON element (defined only when "value" ~= nil)
   -- "path" table reference is the same on each callback invocation, but its content differs every time.
   --    Do not modify "path" array inside your callback function, use it as read-only.
   --    Do not save reference to "path" for future use (create shallow table copy instead).
   -- callback function should return a value, when it is invoked with argument "value" == nil
   --    a truthy value means user wants to decode this JSON object/array and create its Lua counterpart (this may be memory-consuming)
   --    a falsy value (or no value returned) means user wants to traverse through this JSON object/array
   --    (returned value is ignored when callback function is invoked with value ~= nil)

   -- Traverse examples:

   --    json.traverse([[ 42 ]], callback)
   --    will invoke callback 1 time:
   --                 path        json_type  value           pos  pos_last
   --                 ----------  ---------  --------------  ---  --------
   --       callback( {},         "number",  42,             2,   3   )
   --
   --    json.traverse([[ {"a":true, "b":null, "c":["one","two"], "d":{ "e":{}, "f":[] } } ]], callback)
   --    will invoke callback 9 times:
   --                 path        json_type  value           pos  pos_last
   --                 ----------  ---------  --------------  ---  --------
   --       callback( {},         "object",  nil,            2,   nil )
   --       callback( {"a"},      "boolean", true,           7,   10  )
   --       callback( {"b"},      "null",    json.null,      17,  20  )   -- special Lua value for JSON null
   --       callback( {"c"},      "array",   nil,            27,  nil )
   --       callback( {"c", 1},   "string",  "one",          28,  32  )
   --       callback( {"c", 2},   "string",  "two",          34,  38  )
   --       callback( {"d"},      "object",  nil,            46,  nil )
   --       callback( {"d", "e"}, "object",  nil,            52,  nil )
   --       callback( {"d", "f"}, "array",   nil,            60,  nil )
   --
   --    json.traverse([[ {"a":true, "b":null, "c":["one","two"], "d":{ "e":{}, "f":[] } } ]], callback)
   --    will invoke callback 9 times if callback returns true when invoked for array "c" and object "e":
   --                 path        json_type  value           pos  pos_last
   --                 ----------  ---------  --------------  ---  --------
   --       callback( {},         "object",  nil,            2,   nil )
   --       callback( {"a"},      "boolean", true,           7,   10  )
   --       callback( {"b"},      "null",    json.null,      17,  20  )
   --       callback( {"c"},      "array",   nil,            27,  nil )  -- this callback returned true (user wants to decode this array)
   --       callback( {"c"},      "array",   {"one", "two"}, 27,  39  )  -- the next invocation brings the result of decoding
   --       callback( {"d"},      "object",  nil,            46,  nil )
   --       callback( {"d", "e"}, "object",  nil,            52,  nil )  -- this callback returned true (user wants to decode this object)
   --       callback( {"d", "e"}, "object",  json.empty,     52,  53  )  -- the next invocation brings the result of decoding (special Lua value for empty JSON object)
   --       callback( {"d", "f"}, "array",   nil,            60,  nil )


   -- Both decoder functions json.decode(s) and json.traverse(s, callback) can accept JSON (argument s)
   --    as a "loader function" instead of a string.
   --    This function will be called repeatedly to return next parts (substrings) of JSON.
   --    An empty string, nil, or no value returned from "loader function" means the end of JSON.
   --    This may be useful for low-memory devices or for traversing huge JSON files.


   --- The json.null table allows one to specify a null value in an associative array (which is otherwise
   -- discarded if you set the value with 'nil' in Lua. Simply set t = { first=json.null }
   local null = {"This Lua table is used to designate JSON null value, compare your values with json.null to determine JSON nulls"}
   json.null = setmetatable(null, {
      __tostring = function() return 'null' end
   })

   --- The json.empty table allows one to specify an empty JSON object.
   -- To encode empty JSON array use usual empty Lua table.
   -- Example: t = { empty_object=json.empty, empty_array={} }
   local empty = {}
   json.empty = setmetatable(empty, {
      __tostring = function() return '{}' end,
      __newindex = function() error("json.empty is an read-only Lua table", 2) end
   })

   -----------------------------------------------------------------------------
   -- Private functions
   -----------------------------------------------------------------------------
   local decode
   local decode_scanArray
   local decode_scanConstant
   local decode_scanNumber
   local decode_scanObject
   local decode_scanString
   local decode_scanIdentifier
   local decode_scanWhitespace
   local encodeString
   local isArray
   local isEncodable
   local isConvertibleToString
   local isRegularNumber

   -----------------------------------------------------------------------------
   -- PUBLIC FUNCTIONS
   -----------------------------------------------------------------------------
   --- Encodes an arbitrary Lua object / variable.
   -- @param   obj     Lua value (table/string/boolean/number/nil/json.null/json.empty) to be JSON-encoded.
   -- @return  string  String containing the JSON encoding.
   function json.encode(obj)
      -- Handle nil and null values
      if obj == nil or obj == null then
         return 'null'
      end

      -- Handle empty JSON object
      if obj == empty then
         return '{}'
      end

      local obj_type = type(obj)

      -- Handle strings
      if obj_type == 'string' then
         return '"'..encodeString(obj)..'"'
      end

      -- Handle booleans
      if obj_type == 'boolean' then
         return tostring(obj)
      end

      -- Handle numbers
      if obj_type == 'number' then
         assert(isRegularNumber(obj), 'numeric values Inf and NaN are unsupported')
         return math_type(obj) == 'integer' and tostring(obj) or string_format('%.17g', obj)
      end

      -- Handle tables
      if obj_type == 'table' then
         local rval = {}
         -- Consider arrays separately
         local bArray, maxCount = isArray(obj)
         if bArray then
            for i = obj[0] ~= nil and 0 or 1, maxCount do
               table_insert(rval, json.encode(obj[i]))
            end
         else  -- An object, not an array
            for i, j in pairs(obj) do
               if isConvertibleToString(i) and isEncodable(j) then
                  table_insert(rval, '"'..encodeString(i)..'":'..json.encode(j))
               end
            end
         end
         if bArray then
            return '['..table_concat(rval, ',')..']'
         else
            return '{'..table_concat(rval, ',')..'}'
         end
      end

      error('Unable to JSON-encode Lua value of unsupported type "'..obj_type..'": '..tostring(obj))
   end

   local function create_state(s)
      -- Argument s may be "whole JSON string" or "JSON loader function"
      -- Returns "state" object which holds current state of reading long JSON:
      --    part = current part (substring of long JSON string)
      --    disp = number of bytes before current part inside long JSON
      --    more = function to load next substring (more == nil if all substrings are already read)
      local state = {disp = 0}
      if type(s) == "string" then
         -- s is whole JSON string
         state.part = s
      else
         -- s is loader function
         state.part = ""
         state.more = s
      end
      return state
   end

   --- Decodes a JSON string and returns the decoded value as a Lua data structure / value.
   -- @param   s           The string to scan (or "loader function" for getting next substring).
   -- @param   pos         (optional) The position inside s to start scan, default = 1.
   -- @return  Lua object  The object that was scanned, as a Lua table / string / number / boolean / json.null / json.empty.
   function json.decode(s, pos)
      return (decode(create_state(s), pos or 1))
   end

   --- Traverses a JSON string, sends everything to user-supplied callback function, returns nothing
   -- @param   s           The string to scan (or "loader function" for getting next substring).
   -- @param   callback    The user-supplied callback function which accepts arguments (path, json_type, value, pos, pos_last).
   -- @param   pos         (optional) The position inside s to start scan, default = 1.
   function json.traverse(s, callback, pos)
      decode(create_state(s), pos or 1, {path = {}, callback = callback})
   end

   local function read_ahead(state, startPos)
      -- Make sure there are at least 32 bytes read ahead
      local endPos = startPos + 31
      local part = state.part  -- current part (substring of "whole JSON" string)
      local disp = state.disp  -- number of bytes before current part inside "whole JSON" string
      local more = state.more  -- function to load next substring
      assert(startPos > disp)
      while more and disp + #part < endPos do
         --  (disp + 1) ... (disp + #part)  -  we already have this segment now
         --  startPos   ... endPos          -  we need to have this segment
         local next_substr = more()
         if not next_substr or next_substr == "" then
            more = nil
         else
            disp, part = disp + #part, string_sub(part, startPos - disp)
            disp, part = disp - #part, part..next_substr
         end
      end
      state.disp, state.part, state.more = disp, part, more
   end

   local function get_word(state, startPos, length)
      -- 1 <= length <= 32
      if state.more then read_ahead(state, startPos) end
      local idx = startPos - state.disp
      return string_sub(state.part, idx, idx + length - 1)
   end

   local function skip_until_word(state, startPos, word)
      -- #word < 30
      -- returns position after that word (nil if not found)
      repeat
         if state.more then read_ahead(state, startPos) end
         local part, disp = state.part, state.disp
         local b, e = string_find(part, word, startPos - disp, true)
         if b then
            return disp + e + 1
         end
         startPos = disp + #part + 2 - #word
      until not state.more
   end

   local function match_with_pattern(state, startPos, pattern, operation)
      -- pattern must be
      --    "^[some set of chars]+"
      -- returns
      --    matched_string, endPos   for operation "read"   (matched_string == "" if no match found)
      --    endPos                   for operation "skip"
      if operation == "read" then
         local t = {}
         repeat
            if state.more then read_ahead(state, startPos) end
            local part, disp = state.part, state.disp
            local str = string_match(part, pattern, startPos - disp)
            if str then
               table_insert(t, str)
               startPos = startPos + #str
            end
         until not str or startPos <= disp + #part
         return table_concat(t), startPos
      elseif operation == "skip" then
         repeat
            if state.more then read_ahead(state, startPos) end
            local part, disp = state.part, state.disp
            local b, e = string_find(part, pattern, startPos - disp)
            if b then
               startPos = startPos + e - b + 1
            end
         until not b or startPos <= disp + #part
         return startPos
      else
         error("Wrong operation name")
      end
   end

   --- Decodes a JSON string and returns the decoded value as a Lua data structure / value.
   -- @param   state             The state of JSON reader.
   -- @param   startPos          Starting position where the JSON string is located.
   -- @param   traverse          (optional) table with fields "path" and "callback" for traversing JSON.
   -- @param   decode_key        (optional) boolean flag for decoding key inside JSON object.
   -- @return  Lua_object,int    The object that was scanned, as a Lua table / string / number / boolean / json.null / json.empty,
   --                            and the position of the first character after the scanned JSON object.
   function decode(state, startPos, traverse, decode_key)
      local curChar, value, nextPos
      startPos, curChar = decode_scanWhitespace(state, startPos)
      if curChar == '{' and not decode_key then
         -- Object
         if traverse and traverse.callback(traverse.path, "object", nil, startPos, nil) then
            -- user wants to decode this JSON object (and get it as Lua value) while traversing
            local object, endPos = decode_scanObject(state, startPos)
            traverse.callback(traverse.path, "object", object, startPos, endPos - 1)
            return false, endPos
         end
         return decode_scanObject(state, startPos, traverse)
      elseif curChar == '[' and not decode_key then
         -- Array
         if traverse and traverse.callback(traverse.path, "array", nil, startPos, nil) then
            -- user wants to decode this JSON array (and get it as Lua value) while traversing
            local array, endPos = decode_scanArray(state, startPos)
            traverse.callback(traverse.path, "array", array, startPos, endPos - 1)
            return false, endPos
         end
         return decode_scanArray(state, startPos, traverse)
      elseif curChar == '"' then
         -- String
         value, nextPos = decode_scanString(state, startPos)
         if traverse then
            traverse.callback(traverse.path, "string", value, startPos, nextPos - 1)
         end
      elseif decode_key then
         -- Unquoted string as key name
         return decode_scanIdentifier(state, startPos)
      elseif string_find(curChar, "^[%d%-]") then
         -- Number
         value, nextPos = decode_scanNumber(state, startPos)
         if traverse then
            traverse.callback(traverse.path, "number", value, startPos, nextPos - 1)
         end
      else
         -- Otherwise, it must be a constant
         value, nextPos = decode_scanConstant(state, startPos)
         if traverse then
            traverse.callback(traverse.path, value == null and "null" or "boolean", value, startPos, nextPos - 1)
         end
      end
      return value, nextPos
   end

   -----------------------------------------------------------------------------
   -- Internal, PRIVATE functions.
   -- Following a Python-like convention, I have prefixed all these 'PRIVATE'
   -- functions with an underscore.
   -----------------------------------------------------------------------------

   --- Scans an array from JSON into a Lua object
   -- startPos begins at the start of the array.
   -- Returns the array and the next starting position
   -- @param   state       The state of JSON reader.
   -- @param   startPos    The starting position for the scan.
   -- @param   traverse    (optional) table with fields "path" and "callback" for traversing JSON.
   -- @return  table,int   The scanned array as a table, and the position of the next character to scan.
   function decode_scanArray(state, startPos, traverse)
      local array = not traverse and {}  -- The return value
      local elem_index, elem_ready, object = 1
      startPos = startPos + 1
      -- Infinite loop for array elements
      while true do
         repeat
            local curChar
            startPos, curChar = decode_scanWhitespace(state, startPos)
            if curChar == ']' then
               return array, startPos + 1
            elseif curChar == ',' then
               if not elem_ready then
                  -- missing value in JSON array
                  if traverse then
                     table_insert(traverse.path, elem_index)
                     traverse.callback(traverse.path, "null", null, startPos, startPos - 1)  -- empty substring: pos_last = pos - 1
                     table_remove(traverse.path)
                  else
                     array[elem_index] = null
                  end
               end
               elem_ready = false
               elem_index = elem_index + 1
               startPos = startPos + 1
            end
         until curChar ~= ','
         if elem_ready then
            error('Comma is missing in JSON array at position '..startPos)
         end
         if traverse then
            table_insert(traverse.path, elem_index)
         end
         object, startPos = decode(state, startPos, traverse)
         if traverse then
            table_remove(traverse.path)
         else
            array[elem_index] = object
         end
         elem_ready = true
      end
   end

   --- Scans for given constants: true, false or null
   -- Returns the appropriate Lua type, and the position of the next character to read.
   -- @param  state        The state of JSON reader.
   -- @param  startPos     The position in the string at which to start scanning.
   -- @return object, int  The object (true, false or json.null) and the position at which the next character should be scanned.
   function decode_scanConstant(state, startPos)
      local w5 = get_word(state, startPos, 5)
      local w4 = string_sub(w5, 1, 4)
      if w5 == "false" then
         return false, startPos + 5
      elseif w4 == "true" then
         return true, startPos + 4
      elseif w4 == "null" then
         return null, startPos + 4
      end
      error('Failed to parse JSON at position '..startPos)
   end

   --- Scans a number from the JSON encoded string.
   -- (in fact, also is able to scan numeric +- eqns, which is not in the JSON spec.)
   -- Returns the number, and the position of the next character after the number.
   -- @param   state        The state of JSON reader.
   -- @param   startPos     The position at which to start scanning.
   -- @return  number,int   The extracted number and the position of the next character to scan.
   function decode_scanNumber(state, startPos)
      local stringValue, endPos = match_with_pattern(state, startPos, '^[%+%-%d%.eE]+', "read")
      local stringEval = loadstring('return '..stringValue)
      if not stringEval then
         error('Failed to scan number '..stringValue..' in JSON string at position '..startPos)
      end
      return stringEval(), endPos
   end

   --- Scans a JSON object into a Lua object.
   -- startPos begins at the start of the object.
   -- Returns the object and the next starting position.
   -- @param   state       The state of JSON reader.
   -- @param   startPos    The starting position of the scan.
   -- @param   traverse    (optional) table with fields "path" and "callback" for traversing JSON
   -- @return  table,int   The scanned object as a table and the position of the next character to scan.
   function decode_scanObject(state, startPos, traverse)
      local object, elem_ready = not traverse and empty
      startPos = startPos + 1
      while true do
         repeat
            local curChar
            startPos, curChar = decode_scanWhitespace(state, startPos)
            if curChar == '}' then
               return object, startPos + 1
            elseif curChar == ',' then
               startPos = startPos + 1
               elem_ready = false
            end
         until curChar ~= ','
         if elem_ready then
            error('Comma is missing in JSON object at '..startPos)
         end
         -- Scan the key as string or unquoted identifier such as in {"a":1,b:2}
         local key, value
         key, startPos = decode(state, startPos, nil, true)
         local colon
         startPos, colon = decode_scanWhitespace(state, startPos)
         if colon ~= ':' then
            error('JSON object key-value assignment mal-formed at '..startPos)
         end
         startPos = decode_scanWhitespace(state, startPos + 1)
         if traverse then
            table_insert(traverse.path, key)
         end
         value, startPos = decode(state, startPos, traverse)
         if traverse then
            table_remove(traverse.path)
         else
            if object == empty then
               object = {}
            end
            object[key] = value
         end
         elem_ready = true
      end  -- infinite loop while key-value pairs are found
   end

   --- Scans JSON string for an identifier (unquoted key name inside object)
   -- Returns the string extracted as a Lua string, and the position after the closing quote.
   -- @param  state        The state of JSON reader.
   -- @param  startPos     The starting position of the scan.
   -- @return string,int   The extracted string as a Lua string, and the next character to parse.
   function decode_scanIdentifier(state, startPos)
      local identifier, idx = match_with_pattern(state, startPos, '^[%w_%-%$]+', "read")
      if identifier == "" then
         error('JSON String decoding failed: missing key name at position '..startPos)
      end
      return identifier, idx
   end

   -- START SoniEx2
   -- Initialize some things used by decode_scanString
   -- You know, for efficiency
   local escapeSequences = { t = "\t", f = "\f", r = "\r", n = "\n", b = "\b" }
   -- END SoniEx2

   --- Scans a JSON string from the opening quote to the end of the string.
   -- Returns the string extracted as a Lua string, and the position after the closing quote.
   -- @param  state        The state of JSON reader.
   -- @param  startPos     The starting position of the scan.
   -- @return string,int   The extracted string as a Lua string, and the next character to parse.
   function decode_scanString(state, startPos)
      local t, idx, surrogate_pair_started, regular_part = {}, startPos + 1
      while true do
         regular_part, idx = match_with_pattern(state, idx, '^[^"\\]+', "read")
         table_insert(t, regular_part)
         local w6 = get_word(state, idx, 6)
         local c = string_sub(w6, 1, 1)
         if c == '"' then
            return table_concat(t), idx + 1
         elseif c == '\\' then
            local esc = string_sub(w6, 2, 2)
            if esc == "u" then
               local n = tonumber(string_sub(w6, 3), 16)
               if not n then
                  error("String decoding failed: bad Unicode escape "..w6.." at position "..idx)
               end
               -- Handling of UTF-16 surrogate pairs
               if n >= 0xD800 and n < 0xDC00 then
                  surrogate_pair_started, n = n
               elseif n >= 0xDC00 and n < 0xE000 then
                  n, surrogate_pair_started = surrogate_pair_started and (surrogate_pair_started - 0xD800) * 0x400 + (n - 0xDC00) + 0x10000
               end
               if n then
                  -- Convert unicode codepoint n (0..0x10FFFF) to UTF-8 string
                  local x
                  if n < 0x80 then
                     x = string_char(n % 0x80)
                  elseif n < 0x800 then
                     -- [110x xxxx] [10xx xxxx]
                     x = string_char(0xC0 + (math_floor(n/64) % 0x20), 0x80 + (n % 0x40))
                  elseif n < 0x10000 then
                     -- [1110 xxxx] [10xx xxxx] [10xx xxxx]
                     x = string_char(0xE0 + (math_floor(n/64/64) % 0x10), 0x80 + (math_floor(n/64) % 0x40), 0x80 + (n % 0x40))
                  else
                     -- [1111 0xxx] [10xx xxxx] [10xx xxxx] [10xx xxxx]
                     x = string_char(0xF0 + (math_floor(n/64/64/64) % 8), 0x80 + (math_floor(n/64/64) % 0x40), 0x80 + (math_floor(n/64) % 0x40), 0x80 + (n % 0x40))
                  end
                  table_insert(t, x)
               end
               idx = idx + 6
            else
               table_insert(t, escapeSequences[esc] or esc)
               idx = idx + 2
            end
         else
            error('String decoding failed: missing closing " for string at position '..startPos)
         end
      end
   end

   --- Scans a JSON string skipping all whitespace from the current start position.
   -- Returns the position of the first non-whitespace character.
   -- @param   state      The state of JSON reader.
   -- @param   startPos   The starting position where we should begin removing whitespace.
   -- @return  int,char   The first position where non-whitespace was encountered, non-whitespace char.
   function decode_scanWhitespace(state, startPos)
      while true do
         startPos = match_with_pattern(state, startPos, '^[ \n\r\t]+', "skip")
         local w2 = get_word(state, startPos, 2)
         if w2 == '/*' then
            local endPos = skip_until_word(state, startPos + 2, '*/')
            if not endPos then
               error("Unterminated comment in JSON string at "..startPos)
            end
            startPos = endPos
         else
            local next_char = string_sub(w2, 1, 1)
            if next_char == '' then
               error('Unexpected end of JSON')
            end
            return startPos, next_char
         end
      end
   end

   --- Encodes a string to be JSON-compatible.
   -- This just involves backslash-escaping of quotes, slashes and control codes
   -- @param   s        The string to return as a JSON encoded (i.e. backquoted string)
   -- @return  string   The string appropriately escaped.
   local escapeList = {
         ['"']  = '\\"',
         ['\\'] = '\\\\',
         ['/']  = '\\/',
         ['\b'] = '\\b',
         ['\f'] = '\\f',
         ['\n'] = '\\n',
         ['\r'] = '\\r',
         ['\t'] = '\\t',
         ['\127'] = '\\u007F'
   }
   function encodeString(s)
      if type(s) == 'number' then
         s = math_type(s) == 'integer' and tostring(s) or string_format('%.f', s)
      end
      return string_gsub(s, ".", function(c) return escapeList[c] or c:byte() < 32 and string_format('\\u%04X', c:byte()) end)
   end

   -- Determines whether the given Lua type is an array or a table / dictionary.
   -- We consider any table an array if it has indexes 1..n for its n items, and no other data in the table.
   -- I think this method is currently a little 'flaky', but can't think of a good way around it yet...
   -- @param   t                 The table to evaluate as an array
   -- @return  boolean,number    True if the table can be represented as an array, false otherwise.
   --                            If true, the second returned value is the maximum number of indexed elements in the array.
   function isArray(t)
      -- Next we count all the elements, ensuring that any non-indexed elements are not-encodable
      -- (with the possible exception of 'n')
      local maxIndex = 0
      for k, v in pairs(t) do
         if type(k) == 'number' and math_floor(k) == k and 0 <= k and k <= 1e6 then  -- k,v is an indexed pair
            if not isEncodable(v) then  -- All array elements must be encodable
               return false
            end
            maxIndex = math_max(maxIndex, k)
         elseif not (k == 'n' and v == #t) then  -- if it is n, then n does not hold the number of elements
            if isConvertibleToString(k) and isEncodable(v) then
               return false
            end
         end -- End of k,v not an indexed pair
      end  -- End of loop across all pairs
      return true, maxIndex
   end

   --- Determines whether the given Lua object / table / value can be JSON encoded.
   -- The only types that are JSON encodable are: string, boolean, number, nil, table and special tables json.null and json.empty.
   -- @param   o        The object to examine.
   -- @return  boolean  True if the object should be JSON encoded, false if it should be ignored.
   function isEncodable(o)
      local t = type(o)
      return t == 'string' or t == 'boolean' or t == 'number' and isRegularNumber(o) or t == 'nil' or t == 'table'
   end

   --- Determines whether the given Lua object / table / variable can be a JSON key.
   -- Integer Lua numbers are allowed to be considered as valid string keys in JSON.
   -- @param   o        The object to examine.
   -- @return  boolean  True if the object can be converted to a string, false if it should be ignored.
   function isConvertibleToString(o)
      local t = type(o)
      return t == 'string' or t == 'number' and isRegularNumber(o) and (math_type(o) == 'integer' or math_floor(o) == o)
   end

   local is_Inf_or_NaN = {[tostring(1/0)]=true, [tostring(-1/0)]=true, [tostring(0/0)]=true, [tostring(-(0/0))]=true}
   --- Determines whether the given Lua number is a regular number or Inf/Nan.
   -- @param   v        The number to examine.
   -- @return  boolean  True if the number is a regular number which may be encoded in JSON.
   function isRegularNumber(v)
      return not is_Inf_or_NaN[tostring(v)]
   end

end

return json
