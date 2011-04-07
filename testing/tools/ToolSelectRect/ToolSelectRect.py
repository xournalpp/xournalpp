# Xournal++
#
# Testclass for Undo / Redohandler
#
# @author Xournal Team
# http://xournal.sf.net
#
# @license GPL

from tools.ToolTest import ToolTest
import os

# TODO: remove if implemented
from TestNotImplementedException import TestNotImplementedException

# TODO: test scale, move, move to another page, move outside of pages (delete)
# test with stroke, text, image

class ToolSelectRect(ToolTest):
	def __init__(self, xoj):
		ToolTest.__init__(self, xoj)

	def runTest(self):
		raise TestNotImplementedException()

