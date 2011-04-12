
public interface UndoRedoHandler {
	/**
	 * If there is something to undo
	 * @return true if yes
	 */
	boolean canUndo();

	/**
	 * If there is something to redo
	 * @return true if yes
	 */
	boolean canRedo();

	/**
	 * Undo the last operation
	 */
	void undo();
	
	/**
	 * Redo the last operation
	 */
	void redo();
	
	/**
	 * Return the name of the item on top of the undo stack
	 */
	String getUndoItemTypeOnStack();
	
	/**
	 * Return the name of the item on top of the redo stack
	 */
	String getRedoItemTypeOnStack();
}
