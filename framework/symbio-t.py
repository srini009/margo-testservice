#!/usr/bin/env python
# (C) 2015 The University of Chicago
#
# See COPYRIGHT in top-level directory.

from enum import Enum
	
class NetworkMicroservice:
	functions = ["network_do_work"]
	def __init__(self, num_providers):
		self.num_providers = num_providers
	
class MemoryMicroservice:
	functions = ["memory_do_work"]
	def __init__(self, num_providers):
		self.num_providers = num_providers

class ComputeMicroservice:
	functions = ["compute_do_work"]
	def __init__(self, num_providers):
		self.num_providers = num_providers

class StorageMicroservice:
	functions = ["storage_do_work"]
	def __init__(self, num_providers):
		self.num_providers = num_providers

class AccessPattern(Enum):
	Fixed = 1
	Dynamic = 2

class OperationTree:
	def __init__(self, microservice_function, first=None, second=None, third=None):
		self.microservice_function = microservice_function
		self.first = first
		self.second = second
		self.third = third

	def traverseTree(self, accessPattern=None):
		print '{'
		print '"val": ' + '"'+str(self.microservice_function) + '"'
		if accessPattern != None:
			print ',' + '"accessPattern": ' + '"'+ str(accessPattern) + '"'

		if self.first != None or self.second != None or self.third != None:
			print ',' + '"children": ['
			if self.first != None:
				self.first[0].traverseTree(self.first[1])
			if self.second != None:
				self.second[0].traverseTree(self.second[1])
			if self.third != None:
				self.third[0].traverseTree(self.third[1])
			print ']'
		print "},"
		


class OperationType:
	def __init__(self, name, tree):
		self.name = name
		self.opTree = tree

class Service:
	def __init__(self):
		self.microservices = list()
		self.opTypes = set()
	
	def addMicroservice(microservice):
		self.microservices.append(microservice)

	def addOperationType(op):
		self.opTypes.append(op)


a = OperationTree(NetworkMicroservice.functions[0], None, None, None)
b = OperationTree(ComputeMicroservice.functions[0], (a, AccessPattern.Fixed), (a, AccessPattern.Dynamic), None)
c = OperationTree(StorageMicroservice.functions[0], (b, AccessPattern.Dynamic), None, None)

c.traverseTree()
