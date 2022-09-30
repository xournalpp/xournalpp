#
# Get basic informations about current git repo (if any)
#
# Variables:
#   GIT_ORIGIN_URL    origin URL of current git repo
#   GIT_ORIGIN_OWNER  origin repo owner
#   GIT_ORIGIN_REPO   origin repo name
#   GIT_BRANCH        current git branch
#
#   PROJECT_BUGREPORT URL to git issue tracker basing on origin remote (if git not found set default tracker)
#
#
# Copyright (c) 2015, Marek Pikuła <marek@pikula.co>
# All rights reserved.
#
# Distributed under the OSI-approved BSD License (the "License") see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the License for more information.
#

find_program (PATH_GIT git)
if (EXISTS "${PROJECT_SOURCE_DIR}/.git" AND PATH_GIT)

  execute_process (COMMAND "${PATH_GIT}" rev-parse --abbrev-ref HEAD
                   WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                   OUTPUT_VARIABLE GIT_BRANCH
                   OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process (COMMAND "${PATH_GIT}" remote get-url origin
                   WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                   OUTPUT_VARIABLE GIT_ORIGIN_URL
                   OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Remove ssh repo host prefix
  string (REGEX REPLACE ".*:" "" GIT_ORIGIN_URL_NOSSH "${GIT_ORIGIN_URL}")
  string (REPLACE "/" ";" GIT_ORIGIN_URL_LIST "${GIT_ORIGIN_URL_NOSSH}")

  if (GIT_ORIGIN_URL_LIST)
    list (GET GIT_ORIGIN_URL_LIST -2 GIT_ORIGIN_OWNER)
    list (GET GIT_ORIGIN_URL_LIST -1 GIT_ORIGIN_REPO_PRE)
    string (FIND "${GIT_ORIGIN_REPO_PRE}" "." GIT_ORIGIN_REPO_DOT REVERSE)
    string (SUBSTRING "${GIT_ORIGIN_REPO_PRE}" 0 ${GIT_ORIGIN_REPO_DOT} GIT_ORIGIN_REPO)
  else ()
    set (GIT_ORIGIN_OWNER "xournalpp")
    set (GIT_ORIGIN_REPO "xournalpp")
  endif ()

  set (PROJECT_BUGREPORT "https://github.com/${GIT_ORIGIN_OWNER}/${GIT_ORIGIN_REPO}/issues/new")

else ()

  set (PROJECT_BUGREPORT "https://github.com/xournalpp/xournalpp/issues/new")

endif ()
