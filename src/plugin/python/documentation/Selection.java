public interface Selection {
	
	/**
	 * Return true if a selection exists, if return false then all other methods will fail
	 */
	boolean existsSelection();
	
	/**
	 * Return the X coordinate relative to the page
	 */
	double getX();
	
	/**
	 * Return the Y coordinate relative to the page
	 */
	double getY();
	
	/**
	 * Sets the X coordinate for the selection (move the selection) relative to the page
	 */
	void setX(double x);
	
	/**
	 * Sets the Y coordinate for the selection (move the selection) relative to the page
	 */
	void setY(double y);
	
	/**
	 * Returns the page which the selection refer to
	 */
	int getPageId();
}
