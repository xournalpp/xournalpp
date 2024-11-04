# Quick-shape-library
## Quick Shape Library along with Extract Stroke Info

### Installation Steps

1. **Place the folder** (`Quick Shape Library`) in:
   - `C:\Users\<user>\AppData\Local\xournalpp\plugins\` on Windows
   - *Note: The `AppData` folder may be hidden.*

3. **Place the icons** (`shapes_symbolic` and `extract-info-symbolic`) in:
   - `C:\Program Files\Xournal++\share\icons\hicolor\scalable\actions` on windows,
   - `~/.local/share/icons/hicolor/scalable/actions/` on Linux

5. **In the Xournal++ app**:
- Go to `Customize Toolbars` > `Plugins`, and place the icons at a suitable location.

4. **Use the plugin** as needed.

---

### Add Your Own Shape with "Extract_Stroke_Info"

1. **Draw your own shape** and place it at the top left corner of the page.
- *Note: The shape will be inserted at the same position. Whrn you want to insert the sape if the page is smaller than the current one, you may not be able to see the inserted shapes.*

2. **Select your shape** and click the `ESI` icon or select the plugin from the `plugin` menu.

3. **Save the shape**:
- It will be saved in the plugin folder as:
  ```
  giveMeName_and_placeMe_in_shapesFolder.lua
  ```
- Rename it and move it to the `Shapes` subfolder.

4. **Coding step**:
- Open `quick_shape_library.lua` in a code editor.
- Focus on **"The dictionary for shapes"** section. You should be able to understand the necessary modifications.

---

### Share Your Shapes!
Don't forget to share your unique shapes with us!



