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


class CutPaste(CopyPasteTest):
	def __init__(self, xoj):
		CopyPasteTest.__init__(self, xoj)

	def runTest(self):
		raise TestNotImplementedException()

#TODO: test cut paste more times without leaving the selection

