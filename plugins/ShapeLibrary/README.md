# Shape Library

Insert and manage shapes via this plugin.

## Dependencies

The plugin uses the LuaGObject module, which is a recent fork of the lgi module (whose last release was in 2017), and serves for creating the graphical user interface.

LuaGObject comes bundled with Xournal++ on Windows and MacOS. On Linux (outside of flatpak and snap packages) you
are advised to install LuaGObject via luarocks:
```term
luarocks install --lua-version=5.x LuaGObject
```
where 5.x is the Lua version used by `xournalpp`. It can be seen from the output of

```term
ldd xournalpp | grep lua
```
If LuaGObject is not found, the plugin will use the lgi-module, if the latter is found.

## First Steps

1. **Activate the plugin** (`Shape Library`) in menu `Plugins > Plugin Manager`

2. **Place the icon** (`shapes_symbolic.svg` in:

   - `C:\Users\<user>\AppData\Local\share\icons\hicolor\scalable\actions` on Windows,
   - `~/.local/share/icons/hicolor/scalable/actions/` on Linux or MacOS

3. **In Xournal++**:
   Open menu `Edit > Toolbars > Customize`. You will find the copied icon in the `Plugins` section. Place them at a suitable location in the toolbar.

4. **Use the plugin** as needed

## Share Your Shapes!

Don't forget to share your unique shapes with us!
