# Xournal++
#
# Testclass for Undo / Redohandler
#
# @author Xournal Team
# http://xournal.sf.net
#
# @license GPL

from clipboard.CopyPasteTest import CopyPasteTest
import os
import gtk

class CopyText(CopyPasteTest):
	def __init__(self, xoj):
		CopyPasteTest.__init__(self, xoj)

	def runTest(self):
		self.copy()

		clipboard = gtk.clipboard_get()
		text = clipboard.wait_for_text()
		assert(text == 'Hallo')


