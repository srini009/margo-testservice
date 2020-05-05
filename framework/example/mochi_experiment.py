#!/usr/bin/env python
# (C) 2015 The University of Chicago
#
# See COPYRIGHT in top-level directory.

from symbiont import *

def main():
	#Generate operation trees, a.k.a composition of microservice functions
	a = OperationTree(ComputeMicroservice.functions[0])
	d = OperationTree(StorageMicroservice.functions[0])
	b = OperationTree(NetworkMicroservice.functions[0], (a, AccessPattern.Dynamic))
	c = OperationTree(NetworkMicroservice.functions[0], (a, AccessPattern.Fixed), (d, AccessPattern.Dynamic))
	f = OperationTree(MemoryMicroservice.functions[0])
	e = OperationTree(NetworkMicroservice.functions[0], (f, AccessPattern.Fixed))

	op1 = OperationType("op1", b) #b is root of operation tree
	op2 = OperationType("op2", c) #c is root of operation tree	
	op3 = OperationType("op3", e) #e is root of operation Tree

	#Create a service with any name
	s = Service("dummy")

	#Add microservice instances to the service
	s.addMicroservice(NetworkMicroservice(1))
	s.addMicroservice(ComputeMicroservice(1))
	s.addMicroservice(StorageMicroservice(1))
	s.addMicroservice(MemoryMicroservice(1))

	#Add operations that are defined above to the service
	#Remember that a service is defined as a set of composed operation types
	s.addOperationType(op1)
	s.addOperationType(op2)

	t = Service("metoo")
	t.addMicroservice(NetworkMicroservice(2))
	t.addMicroservice(ComputeMicroservice(1))
	t.addMicroservice(StorageMicroservice(1))
	t.addMicroservice(MemoryMicroservice(1))
	t.addOperationType(op3)

	#Create a Mochi experiment, add services to the experiment, and generate the headers
	m = MochiExperiment("test")
	m.addService(s)
	m.addService(t)
	m.generateHeaders()

main()
