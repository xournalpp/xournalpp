#!/usr/bin/env python

import re
import sys
import os.path

# def gather_functions(file_name:str):
def gather_functions(file_name):
    '''
    Gathers the (lua) API functions defined in the given file

        Parameters:
            file_name (str): Path to the file to process

        Yields:
            (c_function_name (str), lua_function_name (str))

        Assumption: lua API is defined via "static const luaL_Reg applib"
    '''
    # define the regex pattern and compile them only once for multiple use later
    start_pattern = re.compile(r'static const luaL_Reg applib\[\] = {')
    main_pattern = re.compile(r'\s*{"(.*)",\s*(applib_.*)},')

    # already capturing lines?
    capture = False

    with open(file_name, 'r') as file:
        for line in file:
            # search for starting pattern (only if not already capturing anyhow)
            if not capture and start_pattern.match(line):
                capture = True
                # make line fit for further detection of an API function
                line = start_pattern.sub("", line, count=1)

            if capture:
                # extract information from lines if matching
                match = main_pattern.match(line)
                if match:
                    groups = match.groups()
                    yield (groups[1], groups[0])

    if not capture:
        raise Exception("luaL_Reg applib not found -> is this file really defining the lua-API?")


# def docs_for_functions(file_name:str, functions: dict[str, str]):
def docs_for_functions(file_name, functions):
    '''
    Search for doc-comments for given functions in given file

        Parameters:
            file_name (str): Path to the file to process
            functions (dict[str,str]): mapping c_function_name -> lua_function_name

        Yields:
            (lua_function_name (str), comments (list[str]), params (list[str]))
    '''
    # define the regex pattern and compile them only once for multiple use later

    # patterns to detect start/end of the comment
    start_pattern = re.compile(r'\s*/\*\*')
    end_pattern = re.compile(r'(.*\*/)')

    # patterns to extract information
    extract_func = re.compile(r'\s*static.*(applib_.*)\(')
    extract_param = re.compile(r'\s*@param\s+([^\s]+)')

    # pattern to replace the C comment prefix (' *') with the one of lua(LS) ('---')
    repl_comment_prefix = re.compile(r'^\s\*\s?')

    # works like a simple state maching -> define possible states here
    STATE_SEARCHING     = 0 # searching for doc-comment
    STATE_COLLECTING    = 1 # collect the doc-comment
    # check if the line following the doc-comment references a function given
    # in 'functions'
    STATE_CHECKING_FUNC = 2

    with open(file_name, 'r') as file:
        # collect the lines with a doc-comment
        comments = list()
        # collect the parameters referenced in the doc-comment
        params = list()
        # start in searching state
        state = STATE_SEARCHING

        for line in file:
            skip_empty = False
            # strip only one trailing newline
            if line[-1] == "\n": line = line[:-1]
            # switch-case over the state
            # ATTENTION: working with if, no elif => continue needed
            if state == STATE_SEARCHING:
                # search for a line starting the doc-comment
                if start_pattern.match(line):
                    state = STATE_COLLECTING
                    # replace the comment prefix
                    line = start_pattern.sub("", line, count=1)
                    skip_empty = True
                    # fallthrough-like to collect this line as usual
                    # 1. line might contain parameter
                    # 2. line might also end the comment as well
            if state == STATE_COLLECTING:
                # check if line ends the comment
                match = end_pattern.match(line)
                if match:
                    skip_empty = True
                    state = STATE_CHECKING_FUNC
                    line = match.groups()[0]
                    line = end_pattern.sub("", line, count=1)
                # check if the line contains the declaration of a parameter
                match = extract_param.search(line)
                if match:
                    params.append(match.groups()[0])
                # collect the line after replacing the leading comment prefix
                line = repl_comment_prefix.sub("", line, count=1)
                if line != "" or not skip_empty:
                    comments.append("--- " + line)
                continue
            if state == STATE_CHECKING_FUNC:
                # check if function given in 'functions' is declared in the line
                match = extract_func.match(line)
                if match and match.groups()[0] in functions:
                    yield (functions[match.groups()[0]], comments, params)
                    # remove function to ensure the doc is only emitted once
                    # and to mark it as processed
                    del functions[match.groups()[0]]
                # re-initialize state variables
                comments = list()
                params = list()
                state = STATE_SEARCHING
                continue

        # 'functions' only contains the unprocessed functions -> check if there
        # are any left
        if len(functions) > 0:
            raise Exception(f"doc strings for functions [{', '.join(functions.values())}] missing")

# def fmt_luaLS_def(file, function_name:str, comments:list[str] = [], params:list[str] = []):
def fmt_luaLS_def(file, function_name, comments = [], params = []):
    '''
    Emit code for a luaLS definition to given file for a function

        Parameters:
            file (file): file to which to emit the code
            function_name (str): name of the function to declare
            comments (list[str]): doc-comment for the function
            params (list[str]): parameters of the function in the order defined
                in the doc-comment.

        Note: The order of the parameters in the doc-comment is also being used
            for the ordering of the function's parameter ordering. Therefore,
            you must specify the parameters of the function in the correct order
            in the doc-comment.
    '''
    if len(comments) > 0:
        print("\n".join(comments), file=file)
    print(f"function app.{function_name}({', '.join(params)}) end\n", file=file)

def insertActions(file_name):
    start_pattern = re.compile(r'constexpr\s+const\s+char\*\s+ACTION_NAMES\[\]\s*=\s*{')

    with open(file_name, 'r') as file:
        inside_array = False
        for line in file:
            line = line.strip()

            if not inside_array:
                if re.match(start_pattern, line):
                    inside_array = True
            else:
                if '}' in line:
                    content = line[:line.find('}')]
                    print(f"---| {re.search(r'(".*?")', content).group(1)}", file=f_out)
                    break
                else:
                    print(f"---| {re.search(r'(".*?")', line).group(1)}", file=f_out)

def insertValuesForEnum(name, prefix, file_name):
    with open(file_name, 'r') as f:
        content = f.read()

        # Pattern to match the following two forms (with word boundaries):
        # 1. name{"string1", ...};
        # 2. name = { "string1", ...};
        pattern = rf'\b{re.escape(name)}\s*(?:=\s*)?\{{([^}}]*)\}}\s*;'
        match = re.search(pattern, content, re.MULTILINE | re.DOTALL)

        if not match:
            raise Exception (f"Enum {name} not found")

        # Extract the inside of the braces
        inside = match.group(1)

        # Find all string literals inside the braces
        string_pattern = r'"([^"]*)"'
        matches = re.findall(string_pattern, inside)

        count = 0
        for match in matches:
            print(f"    {prefix}_{match} = {count},", file=f_out)
            count += 1



if __name__ == "__main__":
    # argument handling
    if len(sys.argv) > 4:
        print(f"Usage: {sys.argv[0]} [path to the file to inspect] [output path]")
        quit(-1)
    else:
        if len(sys.argv) >= 1:
            if len(sys.argv) >= 2:
                file_name = sys.argv[1]
            else:
                file_name = "src/core/plugin/luapi_application.h"
            stem, ext = os.path.splitext(file_name)
            assert ext in [".h", "cpp"]
            fn_out = stem + ".def.lua"

            if file_name == "src/core/plugin/luapi_application.h":
                fn_out = "plugins/luapi_application.def.lua"

        if len(sys.argv) >= 3:
            fn_out = sys.argv[2]


    # real work
    with open(fn_out, 'w') as f_out:
        print("---@meta", file=f_out)
        print("app = {}", file=f_out)

        # Add functions
        funcs = dict(gather_functions(file_name))
        funcs_emitted = set()
        for x in docs_for_functions(file_name, funcs):
            funcs_emitted.add(x[0])
            fmt_luaLS_def(f_out, *x)

        for k in sorted(funcs.keys()):
            i = funcs[k]
            if i not in funcs_emitted:
                print(f"Warning: Did not find doc-comments for API function {i}")
                fmt_luaLS_def(f_out, i)

        # Add alias for actions
        print("---@alias Action", file=f_out)
        insertActions("src/core/enums/generated/Action.NameMap.generated.h")
        print("", file=f_out)

        # Add enum values
        print("---@enum", file=f_out)
        print("app.C = {", file=f_out)
        insertValuesForEnum("toolSizeNames", "ToolSize", "src/core/control/ToolEnums.h")
        insertValuesForEnum("toolNames", "Tool", "src/core/control/ToolEnums.h")
        insertValuesForEnum("eraserTypeNames", "EraserType", "src/core/control/ToolEnums.h")
        insertValuesForEnum("orderChangeNames", "OrderChange", "src/core/control/tools/EditSelection.h")
        print("}", file=f_out)
