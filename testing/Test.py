import xournal
import undo.UndoRedoTest
import os

def xournalTest(args = ''):
	print 'Xournal testsuit started...'

	xoj = xournal.Xournal()
	
	xournalRunTestInSubfolder(xoj, 'tools');
	xournalRunTestInSubfolder(xoj, 'undo');


def xournalRunTestInSubfolder(xoj, subfolder):
	path = os.path.realpath(__file__)
	path = os.path.dirname(path)
	folder = os.path.join(path, subfolder)

	print 'Running scripts in %s' % folder

	for name in os.listdir(folder):
		dirfile = os.path.join(folder, name)
		
		if os.path.isdir(dirfile) and not name.startswith('.') and os.path.exists(os.path.join(dirfile, 'Test.py')):
			print 'Run test in %s' % dirfile
			print 'Debug: import %s from %s' % (name, subfolder + '.' + name + '.Test')
			__import__(subfolder + '.' + name + '.Test', fromlist = name)
			cls = globals()[name]
		   inst = cls()
			inst.



#	print xoj.openFile('/home/andreas/tmp/Notiz-10-03-2011-16-57.xoj')

"""	xoj.mousePressed(10, 10)
	xoj.mouseMoved(20, 20)
	xoj.mouseReleased()

	ud = ColorUndoAction()
"""


