#!/usr/bin/env python
# ------------------------------------------------------------------------------#
# Multicore Programming (Labs 2 - 4)                                            #
# httperf Workload Generator                                                    #
# Copyright (c) 2016 - 2017 Dr. Christopher Mitchell                            #
#                                                                               #
# Usage: lab2_workload.py [-n <number of sessions>] [-m <requests per session>] #
# ------------------------------------------------------------------------------#
import os, sys
import argparse

from reqgen import ReqGen

DEFAULT_SESSIONS = 32
DEFAULT_REQS_PER_SESSION = 1024

def main():
	# Parse command line arguments
	parser = argparse.ArgumentParser(description="Generates Lab 2 httperf test workloads")
	parser.add_argument('-n', '--sessions', required=False, type=int, default=DEFAULT_SESSIONS, \
	                    help='Number of sessions to generate')
	parser.add_argument('-m', '--requests', required=False, type=int, default=DEFAULT_REQS_PER_SESSION, \
	                    help='Number of requests to generate per session')
	parser.add_argument('-z', '--zipfian', action='store_true', \
	                    help='Generate a Zipfian, not uniform, distribution of keys')
	args = parser.parse_args()

	# Generate workload
	req_gen = ReqGen()
	for i in xrange(args.sessions):
		for j in xrange(args.requests):
			sys.stdout.write(req_gen.genRequest(args.zipfian))
		sys.stdout.write("\n")

if __name__ == "__main__":
	main()
