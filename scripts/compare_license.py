"""Script for copyright/license reports

Dependencies:
 - python3
 - ripgrep

Assumptions:
 - copyright.txt file is in CWD
 - copyright.txt is in https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/ format
 - all relevant files are within CWD and not ignored by git (ripgrep default behaviour)

For xournalpp execute using:

    python3 scripts/compare_license.py


Workflow:

 1. Run the script
 2. In case script exits with status 1 adapt copyright.txt or this script (see comments I, II, III in below code)
 3. Rerun script now it should exit with status 0

Note: This script cannot automatically detect whether you added a file that should be licensed differently
but does not indicate this in any way. Please refer to comment II in the code below and add it to the whitelist.
"""

from typing import Set
import re
import os
import subprocess

def get_files_from_copyright_format(file: str) -> Set[str]:
    """Get all Files listed in a copyright file

    Args:
     - file: file formatted according to https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
    """
    with open('copyright.txt', 'r') as f:
        lines = f.readlines()

    files = set()
    for l in lines:
        if re.match("^Files: |^ {7}[a-zA-Z0-9/_\-.*]* *$", l):
            files.add(l[7:].strip())
    return files

def get_all_files():
    stdout = os.popen('rg --files').readlines()
    files = [f.strip() for f in stdout]
    return set(files)

def get_files_containing_copyright_or_license():
    """Find all files containing either
     - copyright
     - license
    (case insensitive)
    Exluding .po files as they create only false positives
    """
    stdout = os.popen('rg -i -e "copyright" -e "license" -l | rg -v "\.po"').readlines()
    files = [f.strip() for f in stdout]
    lc_files = set(files)

    stdout = os.popen('rg --files-without-match "@author Xournal\+\+"').readlines()
    files = [f.strip() for f in stdout]
    xpp_files = set(files)
    return (lc_files & xpp_files)

def get_changed_files_since(git_hash:str):
    stdout = os.popen(f'git diff {git_hash} HEAD --name-only').readlines()
    files = [f.strip() for f in stdout]
    return set(files)

def get_source_files_missing_license_of_header(scanned_files:Set[str], all_files:Set[str]) -> Set[str]:
    """Return all `.cpp` files which do not have a license but their corrsponding `.h` file has.

    Args:
        scanned_files (Set[str]): Files which have a license header
        all_files (Set[str]): all Files in the project (used for existence check)
    """
    scanned_header_files = set()
    scanned_source_files = set()
    for f in scanned_files:
        if f.endswith(".h"):
            scanned_header_files.add(f.strip('.h'))
        elif f.endswith('.cpp'):
            scanned_source_files.add(f.strip('.cpp'))

    missing_source_files = scanned_header_files - scanned_source_files
    source_file_exists = lambda x: (x+'.cpp') in all_files
    return set(filter(source_file_exists, missing_source_files))

# I: Add an entry if a file is detected automatically as a file with special
# license/copyright, but which is actually licensed/copyrighted under the same
# license/copyright as xournalpp.
# Please add a short comment explaining why it's whitelisted
def get_whitelist_not_listed():
    """Whitelist for files containing the searched for substrings but
    are not necessary for the copyright.txt"""
    white_list = set()
    white_list.add("ABOUT-NLS") # false positive
    white_list.add("copyright.txt") # copyright/license summary file
    white_list.add("scripts/compare_license.py") # this very script
    white_list.add("CMakeLists.txt") # false positive
    white_list.add("LICENSE") # main license file
    white_list.add("rpm/fedora/xournalpp.spec") # false positive
    white_list.add("windows-setup/xournalpp.nsi") # false positive
    white_list.add("ui/about.glade") # false positive
    white_list.add("src/win32/xpp.rc.in") # false positive
    return white_list

# II: Add an entry to the whitelist if you added a file which has special
# licensing/copyright but does not contain any of the substrings used to 
# automatically identify such files
# The rational should be explained in the copyright.txt file itself.
# Do not use comments in this file to explain the rational.
def get_whitelist_not_found():
    """Whitelist for files listed in copyright.txt but do not include 
    the searched for substrings"""
    white_list = set()
    white_list.add("*")
    white_list.add("debian/changelog")
    white_list.add("debian/compat")
    white_list.add("debian/control")
    white_list.add("debian/docs")
    white_list.add("debian/package_description")
    white_list.add("debian/rules")
    white_list.add("debian/source/format")
    white_list.add("ui/pixmaps/application-x-xojpp.svg")
    white_list.add("ui/pixmaps/application-x-xopp.svg")
    white_list.add("ui/pixmaps/application-x-xopt.svg")
    white_list.add("ui/pixmaps/com.github.xournalpp.xournalpp.png")
    white_list.add("ui/pixmaps/com.github.xournalpp.xournalpp.svg")
    white_list.add("ui/pixmaps/gnome-mime-application-x-xopp.svg")
    white_list.add("ui/pixmaps/gnome-mime-application-x-xopt.svg")
    white_list.add("ui/pixmaps/xopt.svg")
    white_list.add("ui/iconsColor-dark/*")
    white_list.add("ui/iconsColor-light/*")
    return white_list

# III: Update git commit hash to current commit once you checked
# that the changes do not affect the licensing information in copyright.txt
last_checked_git_commit_hash = "89eee20c3dab1d1bfc18d125f32d510b6862168d"

changed_files = get_changed_files_since(last_checked_git_commit_hash)

summary_files = get_files_from_copyright_format("copyright.txt")
scanned_files = get_files_containing_copyright_or_license()

found = summary_files & scanned_files
not_found = summary_files - scanned_files - get_whitelist_not_found()
not_listed = scanned_files - summary_files - get_whitelist_not_listed()

# Copyright could change with the same commit. Hence, it needs to be exluded.
all_whitelisted = (get_whitelist_not_found() | get_whitelist_not_listed()) - set(["copyright.txt"])
# Files inside copyright.txt or mentioned in whitelist should be checked for
# diffs affecting the license/copyright
out_of_date = (all_whitelisted | summary_files) & changed_files

missing_source_license = get_source_files_missing_license_of_header(scanned_files, get_all_files())

print("Found License/Copyright both in copyright.txt and repo: ",len(found))
if not_listed:
    print()
    print("No License/Copyright listed in copyright.txt (but found in repo):")
    for f in sorted(not_listed):
        print(" ", f)
else:
    print("- All automatically detected files listed or whitelisted")


if not_found:
    print()
    print("No License/Copyright found in repo (but listed in copyright.txt):")
    for f in sorted(not_found):
        print(" ", f)
else:
    print("- All listed files automatically detected or whitelisted")


if out_of_date:
    print()
    print("Following items are whitelisted or listed in copyright.txt but changed since last check:")
    for f in sorted(out_of_date):
        print(" ", f)
else:
    print("- No listed file got changed since the last check.")

if missing_source_license:
    print()
    print("Following `.cpp` files do NOT contain a license even though their accompanying `.h` file does.")
    for f in sorted(missing_source_license):
        print(" ", f)

if not_found or not_listed:
    print("⚠️ Update required")
    exit(1)

if out_of_date:
    "⚠️ Recheck required"
    exit(1)

if missing_source_license:
    "⚠️ Adding license header required"
    exit(1)

print("🎉 Success")
exit(0)
