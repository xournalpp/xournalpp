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

class CopyImage(CopyPasteTest):
	def __init__(self, xoj):
		CopyPasteTest.__init__(self, xoj)

	def runTest(self):
		self.copy()

		clipboard = gtk.clipboard_get()
		img = clipboard.wait_for_image()
		assert(img != None)

		# TODO: check image
		img.save('/tmp/xoj.png', 'png')

