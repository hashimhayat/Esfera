#!/usr/bin/env python
# ---------------------------------------------------
# Multicore Programming (Labs 2 - 4)
# Request Generator Component
# Copyright (c) 2016 Dr. Christopher Mitchell
# ---------------------------------------------------

import random
import string
import math

ZIPF_CONSTANT = 1.1
MIN_VAL_LEN = 256
MAX_VAL_LEN = 4096

class ReqGen:
	def __init__(self):
		self.zipf_gen = ZipfGenerator(ZIPF_CONSTANT)

	def genRequest(self, zipfian = False):
		req_type = random.randint(0, 9)
		key = self.genKey(zipfian)
	
		if req_type < 1:
			req = self.genInsert(key)
		elif req_type < 2:
			req = self.genDelete(key)
		else:
			req = self.genLookup(key)
	
		# Extra newline to force httperf to open new connection
		return req
	
	def genInsert(self, key):
		val = self.genValue()
		output = "/%s method=POST contents='%s' think=0\n" % (key, val)
		return output
	
	def genDelete(self, key):
		output = "/%s method=DELETE think=0\n" % (key)
		return output
	
	def genLookup(self, key):
		output = "/%s method=GET think=0\n" % (key)
		return output
	
	def genKey(self, zipfian = False):
		if zipfian:
			return "user%d" % (self.zipf_gen.next() % 1e7)
		else:
			return "user%d" % (random.randint(0,1e7))
	
	def genValue(self):
		val_len = random.randint(MIN_VAL_LEN, MAX_VAL_LEN)
		return "".join([random.choice(string.letters) for i in xrange(val_len)])
	
# based on code written in fortran-90 from:
# http://users.bigpond.net.au/amiller/random.html
#
# which in turn references:
#
#! Algorithm from page 551 of:
#! Devroye, Luc (1986) `Non-uniform random variate generation',
#! Springer-Verlag: Berlin.   ISBN 3-540-96305-7 (also 0-387-96305-7)

class ZipfGenerator:
    def __init__(self, a):
        if a <= 1.0:
            raise ValueError, "<a> must be > 1.0"
        else:
            self.a = a
        self.b = 2.0 ** (a - 1.0)
        self.const = -1.0 / (a - 1.0)

    def next(self):
        while 1:
            u = random.random()
            v = random.random()
            r = math.floor (u ** self.const)
            t = (1.0 + (1.0/r)) ** (self.a - 1.0)
            if (v * r * (t - 1.0) / (self.b - 1.0) <= t/self.b):
                return r

