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

# Known limitation: This script cannot automatically detect whether you added a file that
# should be licensed differently but does not indicate this in any way. Please refer to
# comment II (see get_whitelist_not_found) and add it to the whitelist if needed.
"""

from typing import Set
import re
import os


def _run_command_and_get_files(command: str) -> Set[str]:
    """Execute a shell command and return stripped file paths as a set."""
    stdout = os.popen(command).readlines()
    return {f.strip() for f in stdout}


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


def get_all_files() -> Set[str]:
    """Get all files tracked by git."""
    return _run_command_and_get_files('rg --files')


def get_files_containing_copyright_or_license() -> Set[str]:
    """Find all files containing either
     - copyright
     - license
    (case insensitive)
    Exluding .po files as they create only false positives
    """
    lc_files = _run_command_and_get_files('rg -i -e "copyright" -e "license" -l | rg -v "\\.po"')
    xpp_files = _run_command_and_get_files('rg --files-without-match "@author Xournal\\+\\+"')
    return lc_files & xpp_files


def get_changed_files_since(git_hash: str) -> Set[str]:
    """Get files changed since the given git hash."""
    return _run_command_and_get_files(f'git diff {git_hash} HEAD --name-only')


def get_source_files_missing_license_of_header(scanned_files: Set[str], all_files: Set[str]) -> Set[str]:
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
    return {
        "ABOUT-NLS",  # false positive
        "copyright.txt",  # copyright/license summary file
        "scripts/compare_license.py",  # this very script
        "CMakeLists.txt",  # false positive
        "LICENSE",  # main license file
        "rpm/fedora/xournalpp.spec",  # false positive
        "windows-setup/xournalpp.nsi",  # false positive
        "ui/about.glade",  # false positive
        "src/exe/win32/xpp.rc.in",  # false positive
        "mac-setup/Info.plist",  # false positive
        "src/core/gui/dialog/AboutDialog.cpp",  # false positive
    }

# II: Add an entry to the whitelist if you added a file which has special
# licensing/copyright but does not contain any of the substrings used to
# automatically identify such files
# The rational should be explained in the copyright.txt file itself.
# Do not use comments in this file to explain the rational.
def get_whitelist_not_found():
    """Whitelist for files listed in copyright.txt but do not include
    the searched for substrings"""
    return {
        "*",
        "debian/changelog",
        "debian/compat",
        "debian/control",
        "debian/docs",
        "debian/package_description",
        "debian/rules",
        "debian/source/format",
        "ui/pixmaps/application-x-xojpp.svg",
        "ui/pixmaps/application-x-xopp.svg",
        "ui/pixmaps/application-x-xopt.svg",
        "ui/pixmaps/com.github.xournalpp.xournalpp.png",
        "ui/pixmaps/com.github.xournalpp.xournalpp.svg",
        "ui/pixmaps/gnome-mime-application-x-xopp.svg",
        "ui/pixmaps/gnome-mime-application-x-xopt.svg",
        "ui/pixmaps/xopt.svg",
        "ui/iconsColor-dark/*",
        "ui/iconsColor-light/*",
        "ui/iconsLucide-dark/*",
        "ui/iconsLucide-light/*",
        "ui/iconsColor-dark/hicolor/scalable/actions/xopp-compass.svg",
        "ui/iconsColor-dark/hicolor/scalable/actions/xopp-setsquare.svg",
        "ui/iconsColor-light/hicolor/scalable/actions/xopp-Tselect-pdf-text-area.svg",
        "ui/iconsColor-light/hicolor/scalable/actions/xopp-Tselect-pdf-text-hd.svg",
        "ui/iconsLucide-dark/hicolor/scalable/actions/xopp-compass.svg",
        "ui/iconsLucide-dark/hicolor/scalable/actions/xopp-draw-spline.svg",
        "ui/iconsLucide-dark/hicolor/scalable/actions/xopp-floating-toolbox.svg",
        "ui/iconsLucide-dark/hicolor/scalable/actions/xopp-setsquare.svg",
        "ui/iconsLucide-light/hicolor/scalable/actions/xopp-compass.svg",
        "ui/iconsLucide-light/hicolor/scalable/actions/xopp-draw-spline.svg",
        "ui/iconsLucide-light/hicolor/scalable/actions/xopp-floating-toolbox.svg",
        "ui/iconsLucide-light/hicolor/scalable/actions/xopp-setsquare.svg",
    }

# III: Update git commit hash to current commit once you checked
# that the changes do not affect the licensing information in copyright.txt
last_checked_git_commit_hash = "c00f7b74009716c488bd666fa8ba7587ea0fed2f"

MSG_UPDATE_REQUIRED = "⚠️ Update required"
MSG_RECHECK_REQUIRED = "⚠️ Recheck required"
MSG_LICENSE_HEADER_REQUIRED = "⚠️ Adding license header required"
MSG_SUCCESS = "🎉 Success"


def _print_check_result(header: str, items: Set[str], success_msg: str) -> bool:
    """Print check result and return True if items were found (problem detected)."""
    if items:
        print()
        print(header)
        for f in sorted(items):
            print(" ", f)
        return True
    else:
        print("- " + success_msg)
        return False


changed_files = get_changed_files_since(last_checked_git_commit_hash)

summary_files = get_files_from_copyright_format("copyright.txt")
scanned_files = get_files_containing_copyright_or_license()

found = summary_files & scanned_files
not_found = summary_files - scanned_files - get_whitelist_not_found()
not_listed = scanned_files - summary_files - get_whitelist_not_listed()

# Copyright could change with the same commit. Hence, it needs to be exluded.
all_whitelisted = (get_whitelist_not_found() | get_whitelist_not_listed()) - {"copyright.txt"}
# Files inside copyright.txt or mentioned in whitelist should be checked for
# diffs affecting the license/copyright
out_of_date = (all_whitelisted | summary_files) & changed_files

missing_source_license = get_source_files_missing_license_of_header(scanned_files, get_all_files())

print("Found License/Copyright both in copyright.txt and repo: ", len(found))

problem_detected = False
problem_detected |= _print_check_result(
    "No License/Copyright listed in copyright.txt (but found in repo):",
    not_listed,
    "All automatically detected files listed or whitelisted"
)
problem_detected |= _print_check_result(
    "No License/Copyright found in repo (but listed in copyright.txt):",
    not_found,
    "All listed files automatically detected or whitelisted"
)
problem_detected |= _print_check_result(
    "Following items are whitelisted or listed in copyright.txt but changed since last check:",
    out_of_date,
    "No listed file got changed since the last check."
)
problem_detected |= _print_check_result(
    "Following `.cpp` files do NOT contain a license even though their accompanying `.h` file does.",
    missing_source_license,
    "All source files have proper license headers"
)

if problem_detected:
    print(MSG_UPDATE_REQUIRED)
    exit(1)

if out_of_date:
    print(MSG_RECHECK_REQUIRED)
    exit(1)

if missing_source_license:
    print(MSG_LICENSE_HEADER_REQUIRED)
    exit(1)

print(MSG_SUCCESS)
exit(0)