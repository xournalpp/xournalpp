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

class CopyPaste(CopyPasteTest):
	def __init__(self, xoj):
		CopyPasteTest.__init__(self, xoj)

	def runTest(self):
		self.copy()

		selection = self.xoj.getSelection()
		selX = selection.getX()
		selY = selection.getY()

		assert self.xoj.paste(), 'paste failed'

		#TODO: assert selection.getX() == selX, 'Selection X position wrong'
		#TODO: assert selection.getY() == selY, 'Selection Y position wrong'


		#TODO: check contents

