import xournal
import undo.UndoRedoTest
import os

def xournalTest(args = ''):
	print 'Xournal testsuit started...'

	xoj = xournal.Xournal()
	
	print '=== Xournal Testsuit ===\n'

	xournalRunTestInSubfolder(xoj, 'tools');
	xournalRunTestInSubfolder(xoj, 'undo');

	print '\n=== End Xournal Testsuit ===\n'


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

			try:
				sFrom = subfolder + '.' + name + '.Test'
				moduleObject = __import__(sFrom, globals(), locals(), [name], -1)
				classObject = getattr(moduleObject, name)
				obj = classObject()
				obj.test(xoj)
			except (AssertionError) as e:
				print type(e)     # the exception instance
				print e.args      # arguments stored in .args
				print e
			except (Exception) as e:
				print "Test %s Unexpected error:" % name, type(e)
				print e.args      # arguments stored in .args
				print e



#	print xoj.openFile('/home/andreas/tmp/Notiz-10-03-2011-16-57.xoj')

"""	xoj.mousePressed(10, 10)
	xoj.mouseMoved(20, 20)
	xoj.mouseReleased()

	ud = ColorUndoAction()
"""


