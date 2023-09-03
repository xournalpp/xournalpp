-----------------------------------------------------------------------
--
-- Copyright (c) 2009-12 Scott Vokes <vokes.s@gmail.com>
--
-- Permission is hereby granted, free of charge, to any person
-- obtaining a copy of this software and associated documentation
-- files (the "Software"), to deal in the Software without
-- restriction, including without limitation the rights to use,
-- copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the
-- Software is furnished to do so, subject to the following
-- conditions:
--
-- The above copyright notice and this permission notice shall be
-- included in all copies or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
-- EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
-- OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
-- NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
-- HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
-- WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
-- FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
-- OTHER DEALINGS IN THE SOFTWARE.
--
------------------------------------------------------------------------
--
-- This is a library for randomized testing with Lua.
-- For usage and examples, see README and the test suite.
--
------------------------------------------------------------------------

------------
-- Module --
------------

-- standard libraries used
local debug, io, math, os, string, table =
   debug, io, math, os, string, table

-- required core global functions
local assert, error, ipairs, pairs, pcall, print, setmetatable, tonumber =
   assert, error, ipairs, pairs, pcall, print, setmetatable, tonumber
local fmt, tostring, type = string.format, tostring, type
local unpack = table.unpack or unpack
local getmetatable, rawget, setmetatable, xpcall =
   getmetatable, rawget, setmetatable, xpcall
local exit, next, require = os.exit, next, require

-- Get containing env, using 5.1's getfenv or emulating it in 5.2
local getenv = getfenv or function(level)
   local info = debug.getinfo(level or 2)
   local n, v = debug.getupvalue(info.func, 1)
   assert(n == "_ENV", n)
   return v
end

---Use lhf's random, if available. It provides an RNG with better
-- statistical properties, and it gives consistent values across OSs.
-- http://www.tecgraf.puc-rio.br/~lhf/ftp/lua/#lrandom
local random = pcall(require, "random") and package.loaded.random or nil

-- Use the debug API to get line numbers, if available.
local debug = pcall(require, "debug") and package.loaded.debug or debug

-- Use luasocket's gettime(), luaposix' gettimeofday(), or os.time for
-- timestamps
local now = pcall(require, "socket") and package.loaded.socket.gettime
   or pcall(require, "posix") and package.loaded.posix.gettimeofday and
         function ()
            local t = package.loaded.posix.gettimeofday()
            local s, us = t.sec, t.usec
            return s + us / 1000000
         end
   or os.time

-- Check command line arguments:
-- -v / --verbose, default to verbose_hooks.
-- -s or --suite, only run the named suite(s).
-- -t or --test, only run tests matching the pattern.
local lt_arg = arg

-- #####################
-- # Utility functions #
-- #####################

local function printf(...) print(string.format(...)) end

local function result_table(name)
   return { name=name, pass={}, fail={}, skip={}, err={} }
end

local function combine_results(to, from)
   local s_name = from.name
   for _,set in ipairs{"pass", "fail", "skip", "err" } do
      local fs, ts = from[set], to[set]
      for name,val in pairs(fs) do
         ts[s_name .. "." .. name] = val
      end
   end
end

local function is_func(v) return type(v) == "function" end

local function count(t)
   local ct = 0
   for _ in pairs(t) do ct = ct + 1 end
   return ct
end


-- ###########
-- # Results #
-- ###########

local function msec(t)
   if t and type(t) == "number" then 
      return fmt(" (%.2fms)", t * 1000)
   else
      return ""
   end
end


local RPass = {}
local passMT = {__index=RPass}
function RPass:tostring_char() return "." end
function RPass:add(s, name) s.pass[name] = self end
function RPass:type() return "pass" end
function RPass:tostring(name)
   return fmt("PASS: %s%s%s",
              name or "(unknown)", msec(self.elapsed),
              self.msg and (": " .. tostring(self.msg)) or "")
end


local RFail = {}
local failMT = {__index=RFail}
function RFail:tostring_char() return "F" end
function RFail:add(s, name) s.fail[name] = self end
function RFail:type() return "fail" end
function RFail:tostring(name)
   return fmt("FAIL: %s%s: %s%s%s",
              name or "(unknown)",
              msec(self.elapsed),
              self.reason or "",
              self.msg and (" - " .. tostring(self.msg)) or "",
              self.line and (" (%d)"):format(self.line) or "")
end


local RSkip = {}
local skipMT = {__index=RSkip}
function RSkip:tostring_char() return "s" end
function RSkip:add(s, name) s.skip[name] = self end
function RSkip:type() return "skip" end
function RSkip:tostring(name)
   return fmt("SKIP: %s()%s", name or "unknown",
              self.msg and (" - " .. tostring(self.msg)) or "")
end


local RError = {}
local errorMT = {__index=RError}
function RError:tostring_char() return "E" end
function RError:add(s, name) s.err[name] = self end
function RError:type() return "error" end
function RError:tostring(name)
   return self.msg or
      fmt("ERROR (in %s%s, couldn't get traceback)",
          msec(self.elapsed), name or "(unknown)")
end


local function Pass(t) return setmetatable(t or {}, passMT) end
local function Fail(t) return setmetatable(t, failMT) end
local function Skip(t) return setmetatable(t, skipMT) end
local function Error(t) return setmetatable(t, errorMT) end


-- ##############
-- # Assertions #
-- ##############

---Renamed standard assert.
local checked = 0
local TS = tostring

local function wraptest(flag, msg, t)
   checked = checked + 1
   t.msg = msg
   if debug then
       local info = debug.getinfo(3, "l")
       t.line = info.currentline
   end
   if not flag then error(Fail(t)) end
end

-- @module lunatest
local lunatest = {}
lunatest.VERSION = 0.95

---Fail a test.
-- @param no_exit Unless set to true, the presence of any failures
-- causes the test suite to terminate with an exit status of 1.
function lunatest.fail(msg, no_exit)
   local line
   if debug then
       local info = debug.getinfo(2, "l")
       line = info.currentline
   end
   error(Fail { msg=msg, reason="(Failed)", no_exit=no_exit, line=line })
end


---Skip a test, with a note, e.g. "TODO".
function lunatest.skip(msg) error(Skip { msg=msg }) end


---got == true.
-- (Named "assert_true" to not conflict with standard assert.)
-- @param msg Message to display with the result.
function lunatest.assert_true(got, msg)
   wraptest(got, msg, { reason=fmt("Expected success, got %s.", TS(got)) })
end

---got == false.
function lunatest.assert_false(got, msg)
   wraptest(not got, msg,
            { reason=fmt("Expected false, got %s", TS(got)) })
end

--got == nil
function lunatest.assert_nil(got, msg)
   wraptest(got == nil, msg,
            { reason=fmt("Expected nil, got %s", TS(got)) })
end

--got ~= nil
function lunatest.assert_not_nil(got, msg)
   wraptest(got ~= nil, msg,
            { reason=fmt("Expected non-nil value, got %s", TS(got)) })
end

local function tol_or_msg(t, m)
   if not t and not m then return 0, nil
   elseif type(t) == "string" then return 0, t
   elseif type(t) == "number" then return t, m
   else error("Neither a numeric tolerance nor string")
   end
end


---exp == got.
function lunatest.assert_equal(exp, got, tol, msg)
   tol, msg = tol_or_msg(tol, msg)
   if type(exp) == "number" and type(got) == "number" then
      wraptest(math.abs(exp - got) <= tol, msg,
               { reason=fmt("Expected %s +/- %s, got %s",
                            TS(exp), TS(tol), TS(got)) })
   else
      wraptest(exp == got, msg,
               { reason=fmt("Expected %q, got %q", TS(exp), TS(got)) })
   end
end

---exp ~= got.
function lunatest.assert_not_equal(exp, got, msg)
   wraptest(exp ~= got, msg,
            { reason="Expected something other than " .. TS(exp) })
end

---val > lim.
function lunatest.assert_gt(lim, val, msg)
   wraptest(val > lim, msg,
            { reason=fmt("Expected a value > %s, got %s",
                         TS(lim), TS(val)) })
end

---val >= lim.
function lunatest.assert_gte(lim, val, msg)
   wraptest(val >= lim, msg,
            { reason=fmt("Expected a value >= %s, got %s",
                         TS(lim), TS(val)) })
end

---val < lim.
function lunatest.assert_lt(lim, val, msg)
   wraptest(val < lim, msg,
            { reason=fmt("Expected a value < %s, got %s",
                         TS(lim), TS(val)) })
end

---val <= lim.
function lunatest.assert_lte(lim, val, msg)
   wraptest(val <= lim, msg,
            { reason=fmt("Expected a value <= %s, got %s",
                         TS(lim), TS(val)) })
end

---#val == len.
function lunatest.assert_len(len, val, msg)
   wraptest(#val == len, msg,
            { reason=fmt("Expected #val == %d, was %d",
                         len, #val) })
end

---#val ~= len.
function lunatest.assert_not_len(len, val, msg)
   wraptest(#val ~= len, msg,
            { reason=fmt("Expected length other than %d", len) })
end

---Test that the string s matches the pattern exp.
function lunatest.assert_match(pat, s, msg)
   s = tostring(s)
   wraptest(type(s) == "string" and s:match(pat), msg,
            { reason=fmt("Expected string to match pattern %s, was %s",
                         pat,
                         (s:len() > 30 and (s:sub(1,30) .. "...")or s)) })
end

---Test that the string s doesn't match the pattern exp.
function lunatest.assert_not_match(pat, s, msg)
   wraptest(type(s) ~= "string" or not s:match(pat), msg,
            { reason=fmt("Should not match pattern %s", pat) })
end

---Test that val is a boolean.
function lunatest.assert_boolean(val, msg)
   wraptest(type(val) == "boolean", msg,
            { reason=fmt("Expected type boolean but got %s",
                         type(val)) })
end

---Test that val is not a boolean.
function lunatest.assert_not_boolean(val, msg)
   wraptest(type(val) ~= "boolean", msg,
            { reason=fmt("Expected type other than boolean but got %s",
                         type(val)) })
end

---Test that val is a number.
function lunatest.assert_number(val, msg)
   wraptest(type(val) == "number", msg,
            { reason=fmt("Expected type number but got %s",
                         type(val)) })
end

---Test that val is not a number.
function lunatest.assert_not_number(val, msg)
   wraptest(type(val) ~= "number", msg,
            { reason=fmt("Expected type other than number but got %s",
                         type(val)) })
end

---Test that val is a string.
function lunatest.assert_string(val, msg)
   wraptest(type(val) == "string", msg,
            { reason=fmt("Expected type string but got %s",
                         type(val)) })
end

---Test that val is not a string.
function lunatest.assert_not_string(val, msg)
   wraptest(type(val) ~= "string", msg,
            { reason=fmt("Expected type other than string but got %s",
                         type(val)) })
end

---Test that val is a table.
function lunatest.assert_table(val, msg)
   wraptest(type(val) == "table", msg,
            { reason=fmt("Expected type table but got %s",
                         type(val)) })
end

---Test that val is not a table.
function lunatest.assert_not_table(val, msg)
   wraptest(type(val) ~= "table", msg,
            { reason=fmt("Expected type other than table but got %s",
                         type(val)) })
end

---Test that val is a function.
function lunatest.assert_function(val, msg)
   wraptest(type(val) == "function", msg,
            { reason=fmt("Expected type function but got %s",
                         type(val)) })
end

---Test that val is not a function.
function lunatest.assert_not_function(val, msg)
   wraptest(type(val) ~= "function", msg,
            { reason=fmt("Expected type other than function but got %s",
                         type(val)) })
end

---Test that val is a thread (coroutine).
function lunatest.assert_thread(val, msg)
   wraptest(type(val) == "thread", msg,
            { reason=fmt("Expected type thread but got %s",
                         type(val)) })
end

---Test that val is not a thread (coroutine).
function lunatest.assert_not_thread(val, msg)
   wraptest(type(val) ~= "thread", msg,
            { reason=fmt("Expected type other than thread but got %s",
                         type(val)) })
end

---Test that val is a userdata (light or heavy).
function lunatest.assert_userdata(val, msg)
   wraptest(type(val) == "userdata", msg,
            { reason=fmt("Expected type userdata but got %s",
                         type(val)) })
end

---Test that val is not a userdata (light or heavy).
function lunatest.assert_not_userdata(val, msg)
   wraptest(type(val) ~= "userdata", msg,
            { reason=fmt("Expected type other than userdata but got %s",
                         type(val)) })
end

---Test that a value has the expected metatable.
function lunatest.assert_metatable(exp, val, msg)
   local mt = getmetatable(val)
   wraptest(mt == exp, msg,
            { reason=fmt("Expected metatable %s but got %s",
                         TS(exp), TS(mt)) })
end

---Test that a value does not have a given metatable.
function lunatest.assert_not_metatable(exp, val, msg)
   local mt = getmetatable(val)
   wraptest(mt ~= exp, msg,
            { reason=fmt("Expected metatable other than %s",
                         TS(exp)) })
end

---Test that the function raises an error when called.
function lunatest.assert_error(f, msg)
   local ok, err = pcall(f)
   local got = ok or err
   wraptest(not ok, msg,
            { exp="an error", got=got,
              reason=fmt("Expected an error, got %s", TS(got)) })
end

-- #########
-- # Hooks #
-- #########

local dot_ct = 0
local cols = 70

local iow = io.write

-- Print a char ([.fEs], etc.), wrapping at 70 columns.
local function dot(c)
   c = c or "."
   io.write(c)
   dot_ct = dot_ct + 1
   if dot_ct > cols then
      io.write("\n  ")
      dot_ct = 0
   end
   io.stdout:flush()
end

local function print_totals(r)
   local ps, fs = count(r.pass), count(r.fail)
   local ss, es = count(r.skip), count(r.err)
   if checked == 0 then return end
   local el, unit = r.t_post - r.t_pre, "s"
   if el < 1 then unit = "ms"; el = el * 1000 end
   local elapsed = fmt(" in %.2f %s", el, unit)
   local buf = {"\n---- Testing finished%s, ",
                "with %d assertion(s) ----\n",
                "  %d passed, %d failed, ",
                "%d error(s), %d skipped."}
   printf(table.concat(buf), elapsed, checked, ps, fs, es, ss)
end


---Default behavior.
default_hooks = {
   begin = false,
   begin_suite = function(s_env, tests)
                    iow(fmt("\n-- Starting suite %q, %d test(s)\n  ",
                            s_env.name, count(tests)))
                 end,
   end_suite = false,
   pre_test = false,
   post_test = function(name, res)
                  dot(res:tostring_char())
               end,
   done = function(r)
             print_totals(r)
             for _,ts in ipairs{ r.fail, r.err, r.skip } do
                for name,res in pairs(ts) do
                   printf("%s", res:tostring(name))
                end
             end
          end,
}


---Default verbose behavior.
verbose_hooks = {
   begin = function(res, suites)
              local s_ct = count(suites)
              if s_ct > 0 then
                 printf("Starting tests, %d suite(s)", s_ct)
              end
           end,
   begin_suite = function(s_env, tests)
                    dot_ct = 0
                    printf("-- Starting suite %q, %d test(s)",
                           s_env.name, count(tests))
                 end,
   end_suite =
      function(s_env)
         local ps, fs = count(s_env.pass), count(s_env.fail)
         local ss, es = count(s_env.skip), count(s_env.err)
         dot_ct = 0
         printf("    Finished suite %q, +%d -%d E%d s%d",
                s_env.name, ps, fs, es, ss)
      end,
   pre_test = false,
   post_test = function(name, res)
                  printf("%s", res:tostring(name))
                  dot_ct = 0
               end,
   done = function(r) print_totals(r) end
}

setmetatable(verbose_hooks, {__index = default_hooks })


-- ################
-- # Registration #
-- ################

local suites = {}
local failed_suites = {}

---Check if a function name should be considered a test key.
-- Defaults to functions starting or ending with "test"
local function is_test_key(k)
   return type(k) == "string" and (k:match("^test.*") or k:match("test$"))
end

-- export is_test_key to enable user to customize this matching function
lunatest.is_test_key = is_test_key

local function get_tests(mod)
   local ts = {}
   if type(mod) == "table" then
       for k,v in pairs(mod) do
           if is_test_key(k) and type(v) == "function" then
               ts[k] = v
           end
       end
       ts.setup = rawget(mod, "setup")
       ts.teardown = rawget(mod, "teardown")
       ts.ssetup = rawget(mod, "suite_setup")
       ts.steardown = rawget(mod, "suite_teardown")
       return ts
   end
   return {}
end

---Add a file as a test suite.
-- @param modname The module to load as a suite. The file is
-- interpreted in the same manner as require "modname".
-- Which functions are tests is determined by is_test_key(name). 
function lunatest.suite(modname)
   local ok, err = pcall(
      function()
         local mod, r_err = require(modname)
         table.insert(suites, {name = modname, tests = get_tests(mod)})
      end)
   if not ok then
      print(fmt(" * Error loading test suite %q:\n%s",
                modname, tostring(err)))
      failed_suites[#failed_suites+1] = modname
   end
end


-- ###########
-- # Running #
-- ###########

local ok_types = { pass=true, fail=true, skip=true }

local function err_handler(name)
   return function (e)
             if type(e) == "table" and e.type and ok_types[e.type()] then return e end
             local msg = fmt("\nERROR in %s():\n\t%s", name, tostring(e))
             msg = debug.traceback(msg, 3)
             return Error { msg=msg }
          end
end


local function run_test(name, test, suite, hooks, setup, teardown)
   local result
   if is_func(hooks.pre_test) then hooks.pre_test(name) end
   local t_pre, t_post, elapsed      --timestamps. requires luasocket.
   local ok, err

   if is_func(setup) then
      ok, err = xpcall(function() setup(name) end, err_handler(name))
   else
      ok = true
   end

   if ok then
      t_pre = now()
      ok, err = xpcall(test, err_handler(name))
      t_post = now()
      elapsed = t_post - t_pre

      if is_func(teardown) then
         if ok then
            ok, err = xpcall(function() teardown(name, elapsed) end,
                             err_handler(name))
         else
            xpcall(function() teardown(name, elapsed) end,
                   function(info)
                      print "\n==============================================="
                      local msg = fmt("ERROR in teardown handler: %s", info)
                      print(msg)
                      os.exit(1)
                   end)
         end
      end
   end

   if ok then err = Pass() end
   result = err
   result.elapsed = elapsed

   -- TODO: log tests w/ no assertions?
   result:add(suite, name)

   if is_func(hooks.post_test) then hooks.post_test(name, result) end
end


local function cmd_line_switches(arg)
   local arg = arg or {}
   local opts = {}
   for i=1,#arg do
      local v = arg[i]
      if v == "-v" or v == "--verbose" then opts.verbose=true
      elseif v == "-s" or v == "--suite" then
         opts.suite_pat = arg[i+1]
      elseif v == "-t" or v == "--test" then
         opts.test_pat = arg[i+1]
      end
   end
   return opts
end


local function failure_or_error_count(r)
   local t = 0
   for k,f in pairs(r.err) do
      t = t + 1
   end
   for k,f in pairs(r.fail) do
      if not f.no_exit then t = t + 1 end
   end
   return t
end

local function run_suite(hooks, opts, results, sname, tests)
   local ssetup, steardown = tests.ssetup, tests.steardown
   tests.ssetup, tests.steardown = nil, nil

   if not opts.suite_pat or sname:match(opts.suite_pat) then
      local run_suite = true
      local res = result_table(sname)

      if ssetup then
         local ok, err = pcall(ssetup)
         if not ok or (ok and err == false) then
            run_suite = false
            local msg = fmt("Error in %s's suite_setup: %s", sname, tostring(err))
            failed_suites[#failed_suites+1] = sname
            results.err[sname] = Error{msg=msg}
         end
      end

      if run_suite and count(tests) > 0 then
         local setup, teardown = tests.setup, tests.teardown
         tests.setup, tests.teardown = nil, nil

         if hooks.begin_suite then hooks.begin_suite(res, tests) end
         res.tests = tests
         for name, test in pairs(tests) do
            if not opts.test_pat or name:match(opts.test_pat) then
               run_test(name, test, res, hooks, setup, teardown)
            end
         end
         if steardown then pcall(steardown) end
         if hooks.end_suite then hooks.end_suite(res) end
         combine_results(results, res)
      end
   end
end

---Run all known test suites, with given configuration hooks.
-- @param hooks Override the default hooks.
-- @param opts Override command line arguments.
-- @usage If no hooks are provided and arg[1] == "-v", the verbose_hooks will
-- be used. opts is expected to be a table of command line arguments.
function lunatest.run(hooks, opts)
   -- also check the namespace it's run in
   local opts = opts and cmd_line_switches(opts) or cmd_line_switches(lt_arg)

   -- Make stdout line-buffered for better interactivity when the output is
   -- not going to the terminal, e.g. is piped to another program.
   io.stdout:setvbuf("line")

   if hooks == true or opts.verbose then
      hooks = verbose_hooks
   else
      hooks = hooks or {}
   end

   setmetatable(hooks, {__index = default_hooks})

   local results = result_table("main")
   results.t_pre = now()

   -- If it's all in one test file, check its environment, too.
   local env = getenv(3)
   if env then
      local main_suite = {name = "main", tests = get_tests(env)}
      table.insert(suites, main_suite)
   end

   if hooks.begin then hooks.begin(results, suites) end

   for _,suite in ipairs(suites) do
      run_suite(hooks, opts, results, suite.name, suite.tests)
   end
   results.t_post = now()
   if hooks.done then hooks.done(results) end

   local failures = failure_or_error_count(results)
   if failures > 0 then os.exit(failures) end
   if #failed_suites > 0 then os.exit(#failed_suites) end
end


-- ########################
-- # Randomization basics #
-- ########################

local set_seed
local random_int
local random_bool
local random_float

if random then
   local _r = random.new()

   set_seed = function(s) _r:seed(s) end
   random_int = function(low, high)
      if not high then high = low; low = 0 end
      return _r:value(low, high)
   end
   random_bool = function() return random_int(0, 1) == 1 end
   random_float = function(low, high)
      return random_int(low, high - 1) + _r:value()
   end
else
   set_seed = math.randomseed
   random_bool = function() return math.random(0, 1) == 1 end
   random_float = function(l, h)
      return random_int(l, h - 1) + math.random()
   end
   random_int = function(l, h)
      if not h then h = l; l = 0 end
      return math.random(l, h)
   end
end

-- Lua_number's bits of precision. IEEE 754 doubles have 52.
local function determine_accuracy()
   for i=1,128 do
      if 2^i == (2^i + 1) then return i - 1 end
   end
   return 128   --long long ints?
end
local bits_of_accuracy = determine_accuracy()


-- ##################
-- # Random strings #
-- ##################


-- For valid char classes, see Lua Reference Manual 5.1, p. 77
-- or http://www.lua.org/manual/5.1/manual.html#5.4.1 .
local function charclass(pat)
   local m = {}

   local match, char = string.match, string.char
   for i=0,255 do
      local c = char(i)
      if match(c, pat) then m[#m+1] = c end
   end

   return table.concat(m)
end


-- Return a (() -> random char) iterator from a pattern.
local function parse_pattern(pattern)
   local cs = {}                --charset
   local idx = 1
   local len = string.len(pattern)
   assert(len > 0, "Cannot generate pattern from empty string.")

   local function at_either_end() return #cs == 0 or #cs == len end
   local function slice(i) return string.sub(pattern, i, i) end

   while idx <= len do
      local c = slice(idx)

      if c == "-" then
         if at_either_end() then
            cs[#cs+1] = c    --literal - at start or end
         else                --range
            local low = string.byte(slice(idx-1)) + 1
            local high = string.byte(slice(idx+1))
            assert(low < high, "Invalid character range: " .. pattern)
            for asc=low,high do
               cs[#cs+1] = string.char(asc)
            end
            idx = idx + 1
         end

      elseif c == "%" then
         local nextc = slice(idx + 1)
         cs[#cs+1] = charclass("%" .. nextc)
         idx = idx + 1

      else
         cs[#cs+1] = c
      end
      idx = idx + 1
   end

   cs = table.concat(cs)
   local len = string.len(cs)
   assert(len > 0, "Empty charset")

   return function()
             local idx = random_int(1, len)
             return string.sub(cs, idx, idx)
          end
end


-- Read a random string spec, return a config table.
local function parse_randstring(s)
   local low, high, rest = string.match(s, "([0-9]+),?([0-9]*) (.*)")
   if low then                  --any match
      if high == "" then high = low end
      return { low = tonumber(low),
               high = tonumber(high),
               gen = parse_pattern(rest) }
   else
      local err = "Invalid random string spec: " .. s
      error(err, 2)
   end
end


-- Generate a random string.
-- @usage e.g. "20 listoftwentycharstogenerate" or "10,20 %l".
local function random_string(spec)
   local info = parse_randstring(spec)
   local ct, diff
   diff = info.high - info.low
   if diff == 0 then ct = info.low else
      ct = random_int(diff) + info.low
   end
   
   local acc = {}
   for i=1,ct do
      acc[i] = info.gen(self)
   end
   local res = table.concat(acc)
   assert(res:len() == ct, "Bad string gen")
   return res
end


-- #########################
-- # General random values #
-- #########################

-- Generate a random number, according to arg.
local function gen_number(arg)
   arg = arg or math.huge
   local signed = (arg < 0)
   local float
   if signed then float = (math.ceil(arg) ~= arg) else
      float = (math.floor(arg) ~= arg)
   end

   local f = float and random_float or random_int
   if signed then
      return f(arg, -arg)
   else
      return f(0, arg)
   end
end


-- Create an arbitrary instance of a value.
local function generate_arbitrary(arg)
   local t = type(arg)
   if t == "number" then
      return gen_number(arg)
   elseif t == "function" then
      return arg(gen_number())        -- assume f(number) -> val
   elseif t == "string" then
      return random_string(arg)
   elseif t == "table"  or t == "userdata" then
      assert(arg.__random, t .. " has no __random method")
      -- assume arg.__random(number) -> val
      return arg.__random(gen_number())
   elseif t == "boolean" then
      return random_bool()
   else
      error("Cannot randomly generate values of type " .. t .. ".")
   end
end


local random_test_defaults = {
   count = 100,
   max_failures = 10,
   max_errors = 5,
   max_skips = 50,
   random_bound = 2^bits_of_accuracy,
   seed_limit = math.min(1e13, 2^bits_of_accuracy),
   always = {},
   seed = nil,
   show_progress = true
}


local function random_args(args)
   local as = {}
   for i=1,#args do
      as[i] = generate_arbitrary(args[i])
   end
   return as
end


local function new_seed(limit)
   limit = limit or 1e13
   return random_int(0, limit)
end


local function get_seeds_and_args(t)
   local ss = {}
   for _,r in ipairs(t) do
      if r.seed then
         ss[#ss+1] = fmt("%s %s\n   Seed: %s",
                         r.reason or "", r.msg and ("\n   " .. r.msg) or "", r.seed)
      end
      if r.args then
         for i,arg in ipairs(r.args) do
            ss[#ss+1] = "  * " .. arg
         end
      end
      ss[#ss+1] = ""
   end
   return ss
end


local function run_randtest(seed, f, args, r, limit)
   local try_ct = 0
   while r.tried[seed] and try_ct < 50 do
      seed = new_seed(limit)
      try_ct = try_ct + 1
   end
   if try_ct >= 50 then
      error(Fail { reason = "Exhausted all seeds" })
   end
   set_seed(seed)
   r.tried[seed] = true

   local result
   local r_args = random_args(args)
   local ok, err = pcall(function() f(unpack(r_args)) end)
   if ok then
      result = Pass()
      result.seed = seed
      r.ps[#r.ps+1] = result
   else
      -- So errors in the suite itself get through...
      if type(err) == "string" then error(err) end
      result = err
      result.seed = seed
      local rt = result:type()
      if rt == "pass" then r.ps[#r.ps+1] = result
      elseif rt == "fail" then r.fs[#r.fs+1] = result
      elseif rt == "error" then r.es[#r.es+1] = result
      elseif rt == "skip" then r.ss[#r.ss+1] = result
      else error("unmatched")
      end
   end
   
   seed = new_seed(limit)
   r.ts = r.ts + 1
   local str_args = {}
   -- Convert args to strs (for display) and add to result.
   for i,v in ipairs(r_args) do
      str_args[i] = tostring(v)
   end
   result.args = str_args
   return seed
end


local function report_trial(r, opt)
   if #r.es > 0 then
      local seeds = get_seeds_and_args(r.es)
      error(Fail { reason = fmt("%d tests, %d error(s).\n   %s",
                                  r.ts, #r.es,
                                  table.concat(seeds, "\n   ")),
                   seeds = seeds})
   elseif #r.fs > 0 then
      local seeds = get_seeds_and_args(r.fs)
      error(Fail { reason = fmt("%d tests, %d failure(s).\n   %s",
                                  r.ts, #r.fs,
                                  table.concat(seeds, "\n   ")),
                   seeds = seeds})
   elseif #r.ss >= opt.max_skips then
      error(Fail { reason = fmt("Too many cases skipped.")})
   else
      error(Pass { reason = fmt(": %d cases passed.", #r.ps) })
   end
end


---Set random seed.
lunatest.set_seed = set_seed

---Get a random value low <= x <= high.
lunatest.random_int = random_int

---Get a random bool.
lunatest.random_bool = random_bool

---Get a random float low <= x < high.
lunatest.random_float = random_float

---Get a random string
lunatest.random_string = random_string

---Get a random argument
lunatest.random_args = random_args

---Run a test case with randomly instantiated arguments,
-- running the test function f opt.count (default: 100) times.
-- @param opt A table with options, or just a test name string.<br>
--    opt.count: how many random trials to perform<br>
--    opt.seed: Start the batch of trials with a specific seed<br>
--    opt.always: Always test these seeds (for regressions)<br>
--    opt.show_progress: Whether to print a . after every opt.tick trials.<br>
--    opt.seed_limit: Max seed to allow.<br>
--    opt.max_failures, max_errors, max_skips: Give up after X of each.<br>
-- @param f A test function, run as f(unpack(randomized_args(...)))
-- @param ... the arg specification. For each argument, creates a
--    random instance of that type.<br>
--    boolean: return true or false<br>
--    number n: returns 0 <= x < n, or -n <= x < n if negative.
--              If n has a decimal component, so will the result.<br>
--    string: Specifiedd as "(len[,maxlen]) (pattern)".<br>
--        "10 %l" means 10 random lowercase letters.<br>
--        "10,30 [aeiou]" means between 10-30 vowels.<br>
--    function: Just call (as f()) and return result.<br>
--    table or userdata: Call v.__random() and return result.<br>
-- @usage
function lunatest.assert_random(opt, f, ...)
   local args = { ... }
   if type(opt) == "string" then
      opt = { name=opt }
   elseif type(opt) == "function" then
      table.insert(args, 1, f)
      f = opt
      opt = {}
   end
      
   setmetatable(opt, { __index=random_test_defaults })

   local seed = opt.seed or os.time()
   local r = { ps={}, fs={}, es={}, ss={}, ts=0, tried={} }

   -- Run these seeds every time, for easy regression testing.
   for _,s in ipairs(opt.always) do
      run_randtest(s, f, args, r, opt.seed_limit)
   end

   set_seed(seed)
   local tick = opt.tick or opt.count / 10

   for i=1,opt.count do
      seed = run_randtest(seed, f, args, r, opt.seed_limit)
      if #r.ss >= opt.max_skips or
         #r.fs >= opt.max_failures or
         #r.es >= opt.max_errors then break
      end
      if opt.show_progress and i % tick == 0 then
         dot(".")
      end
   end
   local overall_status = (passed == count and "PASS" or "FAIL")
   
   report_trial(r, opt)
end

-- export module
return lunatest
