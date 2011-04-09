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

class PasteImage(CopyPasteTest):
	def __init__(self, xoj):
		CopyPasteTest.__init__(self, xoj)

	def runTest(self):
		path = os.path.realpath(__file__ + '/../img.png')
		assert os.path.exists(path), "file does not exist"
		image = gtk.gdk.pixbuf_new_from_file(path)

		clipboard = gtk.clipboard_get()
		clipboard.set_image(image)
		clipboard.store()

		assert self.xoj.paste(), 'paste failed'

