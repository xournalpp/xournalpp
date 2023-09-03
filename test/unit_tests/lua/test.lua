local lunatest = require "lunatest"

local assert_true, assert_false = lunatest.assert_true, lunatest.assert_false
local assert_diffvars = lunatest.assert_diffvars
local assert_boolean, assert_not_boolean = lunatest.assert_boolean, lunatest.assert_not_boolean
local assert_len, assert_not_len = lunatest.assert_len, lunatest.assert_not_len
local assert_match, assert_not_match = lunatest.assert_match, lunatest.assert_not_match
local assert_error = lunatest.assert_error
local assert_lt, assert_lte = lunatest.assert_lt, lunatest.assert_lte
local assert_gt, assert_gte = lunatest.assert_gt, lunatest.assert_gte
local assert_nil, assert_not_nil = lunatest.assert_nil, lunatest.assert_not_nil
local assert_equal, assert_not_equal = lunatest.assert_equal, lunatest.assert_not_equal
local assert_string, assert_not_string = lunatest.assert_string, lunatest.assert_not_string
local assert_metatable, assert_not_metatable = lunatest.assert_metatable, lunatest.assert_not_metatable
local assert_userdata, assert_not_userdata = lunatest.assert_userdata, lunatest.assert_not_userdata
local assert_thread, assert_not_thread = lunatest.assert_thread, lunatest.assert_not_thread
local assert_function, assert_not_function = lunatest.assert_function, lunatest.assert_not_function
local assert_table, assert_not_table = lunatest.assert_table, lunatest.assert_not_table
local assert_number, assert_not_number = lunatest.assert_number, lunatest.assert_not_number
local skip, fail = lunatest.skip, lunatest.fail


local sep = package.config:sub(1,1)                                                  -- "/" on Linux and MacOS, "\\" (escaped backslash) on Windows
local sourceDir = debug.getinfo(1).source:sub(2):match("(.*[/\\])"):gsub("/", sep)   -- directory containing the Lua script
local testDoc = sourceDir .. "testDoc.xopp"                                          -- path of the test document

function test_docStructure()
    local success = app.openFile(testDoc)
    assert_true(success)

    app.setCurrentPage(3)
    app.setCurrentLayer(2)
    local doc = app.getDocumentStructure()

    assert_equal(doc["xoppFilename"], testDoc)
    assert_equal(doc["pdfBackgroundFilename"], "")
    assert_equal(doc["currentPage"], 3)

    local pages = doc["pages"]
    assert_equal(#pages, 3)

    assert_equal(pages[3]["currentLayer"], 2)

    assert_true(pages[1]["isAnnotated"])
    assert_false(pages[2]["isAnnotated"])
    assert_true(pages[3]["isAnnotated"])

    assert_equal(#pages[1]["layers"], 1)
    assert_equal(#pages[3]["layers"], 3)
    assert_true(pages[3]["layers"][1]["isAnnotated"])
    assert_true(pages[3]["layers"][2]["isAnnotated"])
    assert_false(pages[3]["layers"][3]["isAnnotated"])

    assert_equal(pages[1]["pageTypeFormat"], "graph")
    assert_equal(pages[2]["pageTypeFormat"], "plain")
    assert_equal(pages[3]["pageTypeFormat"], "ruled")
end

function test_sidebarPage()
    app.setSidebarPageNo(1)
    assert_equal(app.getSidebarPageNo(), 1)
    app.setSidebarPageNo(2)
    assert_equal(app.getSidebarPageNo(), 2)
end

function test_layers()

    function getNumberOfLayers()
        local doc = app.getDocumentStructure()
        local curPage = doc["currentPage"]
        return #doc["pages"][curPage]["layers"]
    end
    function getCurrentLayer()
        local doc = app.getDocumentStructure()
        local curPage = doc["currentPage"]
        return doc["pages"][curPage]["currentLayer"]
    end

    local numLayer, curLayer = getNumberOfLayers(), getCurrentLayer()
    print(numLayer, curLayer)
    app.layerAction("ACTION_NEW_LAYER")
    assert_equal(getNumberOfLayers(), numLayer + 1)
    assert_equal(getCurrentLayer(), curLayer + 1)
    app.layerAction("ACTION_DELETE_LAYER")
    assert_equal(getNumberOfLayers(), numLayer)
    local expectedLayer = curLayer < numLayer and curLayer + 1 or curLayer
    assert_equal(getCurrentLayer(), expectedLayer)
end

lunatest.run()
