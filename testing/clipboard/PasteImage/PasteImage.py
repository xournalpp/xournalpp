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

# TODO: remove if implemented
from TestNotImplementedException import TestNotImplementedException


class PasteImage(CopyPasteTest):
	def __init__(self, xoj):
		CopyPasteTest.__init__(self, xoj)

	def runTest(self):
		raise TestNotImplementedException()

	def copy_image(f):
		 assert os.path.exists(f), "file does not exist"
		 image = gtk.gdk.pixbuf_new_from_file(f)

		 clipboard = gtk.clipboard_get()
		 clipboard.set_image(image)
		 clipboard.store()

