

class TestNotImplementedException(Exception):
	def __init__(self):
		pass


	def __str__(self):
		return 'TestNotImplementedException'
