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

class CutPaste(CopyPasteTest):
	def __init__(self, xoj):
		CopyPasteTest.__init__(self, xoj)

	def runTest(self):
		self.cut()

		path = os.path.realpath(__file__ + '/../empty.xoj')
		self.checkContents(path)

		assert self.xoj.paste(), 'paste failed'

		# clear selection
		self.xoj.setSelectedTool(self.xoj.TOOL_ERASER)

		path = os.path.realpath(__file__ + '/../../source.xoj')
		self.checkContents(path)
