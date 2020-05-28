# -*- coding:utf-8 -*-

import time
import sys
sys.path.append(r"C:\Users\hulinhong\Documents\github\jiffy\build\lib\Debug")  # _elapse.pyd 在这个路径
sys.path.append(r"/home/b/doc/jiffy/build/lib")  # _elapse.so 在这个路径
sys.path.append(r"C:\Users\11452\Documents\github\jiffy\build\Debug")  # _elapse.so 在这个路径
import _elapse


import unittest


def st_time(func):
	"""
		st decorator to calculate the total time of a func
	"""

	def st_func(*args, **keyArgs):
		t1 = time.time()
		r = func(*args, **keyArgs)
		# t2 = time.time()
		print "Function=%s, Time=%s" % (func.__name__, time.time() - t1)
		return r

	return st_func


g_n = 1000


class TimerAddBenchmarkTest(unittest.TestCase):
	# 测试各种延迟回调的insert性能
	N = g_n

	def _cb(self):
		pass

	@st_time
	def testCallbackMgrAdd(self):
		# 项目实现的C++ Timer管理器，在python实现的CallbackMgr基础上，实现了Crontab等复杂的重复定时逻辑
		# begin = time.time()
		mgr = _elapse.CallbackManager()
		for i in xrange(self.N):
			mgr.callback(str(i), 0, self._cb)
		# print 'mgr', time.time() - begin


class TimerAddRemoveBenchmarkTest(unittest.TestCase):
	# 测试各种延迟回调的增删性能
	N = g_n

	def _cb(self):
		pass

	@st_time
	def testCallbackMgrAddRemove(self):
		# 项目实现的C++ Timer管理器，在python实现的CallbackMgr基础上，实现了Crontab等复杂的重复定时逻辑
		mgr = _elapse.CallbackManager()
		for i in xrange(self.N):
			mgr.callback(str(i), i, self._cb)
		mgr.destroy()


class TimerTriggerBenchmarkTest(unittest.TestCase):
	# 测试各种延迟回调的触发性能
	N = 1000

	def __init__(self, methodName='runTest'):
		super(TimerTriggerBenchmarkTest, self).__init__(methodName)
		self._counter = 0

	def setUp(self):
		super(TimerTriggerBenchmarkTest, self).setUp()
		self._counter = 0

	def _cb(self):
		self._counter += 1

	def testCallbackMgrTrigger(self):
		# 项目实现的C++ Timer管理器，在python实现的CallbackMgr基础上，实现了Crontab等复杂的重复定时逻辑
		mgr = _elapse.CallbackManager()
		for i in xrange(self.N):
			mgr.callback(str(i), 0.0, self._cb)
		while self._counter < self.N:
			print "poppppp1"
			_elapse.pop_expires()
			print "poppppp2"

		print "poppppp3"
		mgr.destroy()
		print "poppppp4"

	# def testZeroDelay(self):
	# 	mgr = _elapse.CallbackManager()
	# 	c = [0]
	#
	# 	def _cb2():
	# 		c[0] += 1
	#
	# 	def _cb():
	# 		c[0] += 1
	# 		mgr.callback(str(2), 0, _cb2)
	#
	# 	mgr.callback(str(1), 0.1, _cb)
	#
	# 	for i in xrange(2):
	# 		time.sleep(0.2)
	# 		_elapse.pop_expires()
	# 		self.assertEqual(i + 1, c[0])

	def testScheduleIsStable(self):
		mgr = _elapse.CallbackManager()
		last_idx = [-1]

		def _mkcb(i):
			def _cb(idx=i):
				self.assertGreater(idx, last_idx[0])
				last_idx[0] = idx

			return _cb

		for i in xrange(20):
			mgr.callback_anony(0, _mkcb(i))
		for i in xrange(20, 40):
			mgr.callback(str(i), 0, _mkcb(i))
		time.sleep(0.2)
		_elapse.pop_expires()


# class xxd(object):
# 	# 测试各种延迟回调的触发性能
# 	N = 10
#
# 	def __init__(self):
# 		self._counter = 0
#
# 	def _cb(self):
# 		self._counter += 1
#
# 	def testCallbackMgrTrigger(self):
# 		# 项目实现的C++ Timer管理器，在python实现的CallbackMgr基础上，实现了Crontab等复杂的重复定时逻辑
# 		mgr = _elapse.CallbackManager()
# 		for i in xrange(self.N):
# 			mgr.callback(str(i), 0.0, self._cb)
# 		while self._counter < self.N:
# 			print "poppppp1"
# 			_elapse.pop_expires()
# 			print "poppppp2"
#
# 		print "poppppp3"
# 		mgr.destroy()
# 		print "poppppp4"


if __name__ == '__main__':
	unittest.main()

	# xxdinst = xxd()
	# print 'io'
	# xxdinst.testCallbackMgrTrigger()
	# print 'gg'

