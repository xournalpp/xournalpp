require("socket")
local https = require("ssl.https")
local ltn12 = require("ltn12")
local json = require("json")

function sendRequest(ink, lang, width, height)
  local path = "https://www.google.com/inputtools/request?ime=handwriting&app=mobilesearch&cs=1&oe=UTF-8"

  if #ink > 150 then 
    print("Number of strokes N = " .. #ink .. " is greater than 150, the maximal number of strokes that can be processed. Only the first 150 strokes will be used. \n")
    ink = { table.unpack(ink, 1, 150) }
  end

  local tbl = {
    requests = {
       {
        writing_guide = {
          writing_area_width = width,
          writing_area_height = height
        },
        ink = ink,
        language = lang
      }
    }
  }

  local payload = json.encode(tbl)

  local response_body = { }

  local res, code, response_headers, status = https.request
  {
    url = path,
    method = "POST",
    headers =
    {
      ["Content-Type"] = "application/json",
      ["Content-Length"] = payload:len()
    },
    source = ltn12.source.string(payload),
    sink = ltn12.sink.table(response_body)
  }
  local response = json.decode(response_body[1])
  if (response[1]=="SUCCESS") then
    local alternatives = response[2][1][2]
    -- return lua table containing all alternatives
    return alternatives
  else
    -- return error message
    return { response[1] }
  end
end

