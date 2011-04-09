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
		assert self.xoj.paste(), 'paste failed'

		#TODO: check contents

