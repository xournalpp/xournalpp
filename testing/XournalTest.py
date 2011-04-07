# Xournal++
#
# Testclass "Template"
#
# @author Xournal Team
# http://xournal.sf.net
#
# @license GPL

import math
import gzip

class XournalTest:
	def __init__(self, xoj):
		self.xoj = xoj

	def test(self):
		self.tearUp()
		self.runTest()
		self.tearDown()

	def runTest(self):
		pass

	def tearUp(self):
		pass

	def tearDown(self):
		pass

	def checkContents(self, path):
		tmpfile = '/tmp/tmp.xoj'
		self.diff = ''
		self.xoj.saveFile(tmpfile)
		if self.compareFiles(tmpfile, path) == False:
			raise AssertionError(self.diff)


	# Compare two xournal files
	def compareFiles(self, fileGet, fileExpected):
		f1 = gzip.open(fileGet, 'rb')
		f2 = gzip.open(fileExpected, 'rb')

		c1 = f1.read()
		c2 = f2.read()

		f1.close()
		f2.close()

		l1 = c1.split()
		l2 = c2.split()

		len1 = len(l1)
		len2 = len(l2)

		if len1 != len2:
			self.diff = 'Lenght missmatch: {0} / {1}'.format(len1, len2)
			return False

		for i in range(0, len1):
			d1 = l1[i].strip()
			d2 = l2[i].strip()

			if d1[0:9] == '<preview>' and d2[0:9] == '<preview>':
				continue

			if d1 != d2:
				self.diff = "Line %i differ:\nGet:\t\"%s\"\nExp:\t\"%s\"" % (i, d1, d2)
				return False

		return True

	# do some testinput with the defined stroke
	def doTestInput(self):
		points = [[40, 40]]
		points.append([50, 51]);
		points.append([50, 81]);
		points.append([130, 180]);
		points.append([230, 300]);
		self.mouseInput(points, 2);

	def mouseMove(self, startPoint, endPoint, maxStrokeLen):
		sX = startPoint[0]
		sY = startPoint[1]
		eX = endPoint[0]
		eY = endPoint[1]

#		print 'move from %i/%i to %i/%i' % (sX, sY, eX, eY)

		lineLen = math.hypot(sX - eX, sY - eY)
		i = maxStrokeLen

		x = eX - sX;
		y = eY - sY;

		while(i < lineLen):
			factor = i / lineLen;
			
			rX = x * factor + sX
			rY = y * factor + sY

#			print 'move to %i/%i' % (rX, rY)
			self.xoj.mouseMoved(rX, rY)

			i += maxStrokeLen


		self.xoj.mouseMoved(eX, eY)


	def mouseInput(self, points, maxStrokeLen):
		if len(points) < 2:
			return

		lastPoint = points[0]
		self.xoj.mousePressed(lastPoint[0], lastPoint[0])

		for p in points:
			self.mouseMove(lastPoint, p, maxStrokeLen)
			lastPoint = p

		self.xoj.mouseReleased()




















