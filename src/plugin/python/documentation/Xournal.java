public interface Xournal {
	int TOOL_PEN = 1;
	int TOOL_ERASER = 2;
	int TOOL_HILIGHTER = 3;
	int TOOL_TEXT = 3;
	int TOOL_IMAGE = 4;
	int TOOL_SELECT_RECT = 5;
	int TOOL_SELECT_REGION = 6;
	int TOOL_SELECT_OBJECT = 7;
	int TOOL_VERTICAL_SPACE = 8;
	int TOOL_HAND = 9;

	/**
	 * Sets the current tool
	 * @param tool A constant defined above
	 */
	void setSelectedTool(int tool);
	
	/**
	 * Return the selected tool
	 * @return (See constants TOOL_*)
	 */
	int getSelectedTool();
	
	/**
	 * Sets the tool Color in RGB
	 */
	void setToolColor(int color);
	
	/**
	 * Returns the tool color in RGB
	 */
	int getToolColor();

	/**
	 * Creates a new file
	 * 
	 * @param [optional] force force creation of a new file, loose your unsaved contents
	 * 
	 * @return if a new file was created, false if the user canceled the operation
	 */
	boolean newFile(boolean force);
	
	/**
	 * Save the current document
	 * @param path a complete filename, e.g. /home/myname/test.xoj
	 * @return true if successfully, false if failed
	 */
	boolean saveFile(String path);
	
	/**
	 * Open a xournal file
	 * @param path a complete filename, e.g. /home/myname/test.xoj
	 * @param [optional] scrollToPage The page to scroll or -1 to not scroll
	 * @return true if successfully, false if failed
	 */
	boolean openFile(String path, int scrollToPage);
	
	/**
	 * Press the mouse on the selected View
	 * @param x The X Coordinate
	 * @param y The Y Coordinate
	 */
	void mousePressed(int x, int y);

	/**
	 * Press the mouse on the selected View
	 * @param x The X Coordinate
	 * @param y The Y Coordinate
	 */
	void mouseMoved(int x, int y);

	/**
	 * Press the mouse on the selected View
	 */
	void mouseReleased();
	
	/**
	 * The Undo- / Redohandler
	 */
	UndoRedoHandler getUndoRedoHandler();

	/**
	 * Return the Xournal Document
	 */
	Document getDocument();
	
	/**
	 * Gets the selected page ID (first Page: 0)
	 */
	int getSelectedPage();
	
	/**
	 * Sets the selected page (first Page: 0)
	 */
	void selectPage(int page);
}
