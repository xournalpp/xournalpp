# CMake configuration flags

Details about installation process are on our [wiki](https://github.com/xournalpp/xournalpp/wiki/Installing).

Here you can find complete list of Xournal++ CMake flags (sorted by categories). Advanced settings are marked with *[A]*.


## `DEBUG` – debugging switches (shouldn't be enabled for normal usage), all *[A]* and disabled by default

| Variable name               | Description
| --------------------------- | -----------
| `DEBUG_INPUT`               | Input debugging, e.g. eraser events etc
| `DEBUG_RECOGNIZER`          | Shape recognizer debug: output score etc
| `DEBUG_SHEDULER`            | Scheduler debug: show jobs etc
| `DEBUG_SHOW_ELEMENT_BOUNDS` | Draw a surrounding border to all elements
| `DEBUG_SHOW_PAINT_BOUNDS`   | Draw a border around all painted rects
| `DEBUG_SHOW_REPAINT_BOUNDS` | Draw a border around all repaint rects


## `DEV` – development options, which in most cases should be leaved as they are

| Variable name                  | Default          | Description
| ------------------------------ | ---------------- | -----------
| `DEV_CALL_LOG`                 | OFF              | Call log (can take loooot of disk space and IO!)
| `DEV_CHECK_GTK3_COMPAT` *[A]*  | OFF              | Adds a few compiler flags to check basic GTK3 upgradeability support (still compiles for GTK2!)
| `DEV_ENABLE_GCOV` *[A]*        | OFF              | Build with gcov support
| `DEV_METADATA_FILE` *[A]*      | metadata.ini     | Metadata file name
| `DEV_METADATA_MAX_ITEMS` *[A]* | 50               | Maximal amount of metadata elements
| `DEV_PRINT_CONFIG_FILE` *[A]*  | print-config.ini | Print config file name
| `DEV_SETTINGS_XML_FILE` *[A]*  | settings.xml     | Settings file name
| `DEV_TOOLBAR_CONFIG` *[A]*     | toolbar.ini      | Toolbar config file name
| `DEV_ERRORLOG_DIR` *[A]*       | errorlogs        | Directory where errorlogfiles will be placed


## `EXT` – add dependency basing on precompiled deb packages (UNIX only)

| Variable name | Default | Description
| ------------- | ------- | -----------
| `EXT_GLIBMM`  | OFF     | Glibmm and Gtkmm binaries


## `ENABLE` – basic stable features support

| Variable name        | Default | Description
| -------------------- | ------- | -----------
| `ENABLE_GTEST`       | OFF     | Download and build GoogleTest (if not previously done) and build tests instead of xournalpp application


## `PATH` – here you can specify alternative location of these binaries (there are no defaults)

| Variable name | Description
| ------------- | -----------
| `PATH_AR`     | `ar` is needed to unpack debs (if any of `EXT` packages are added)
| `PATH_GIT`    | `git` is needed to include info about issue tracker and other git-repo-realted info
| `PATH_TAR`    | `tar` is needed to unpack debs (if any of `EXT` packages are added)
| `PATH_WGET`   | `wget` is needed to download debs (if any of `EXT` packages are added)

