#!/usr/bin/env python
# (C) 2015 The University of Chicago
#
# See COPYRIGHT in top-level directory.

from enum import Enum

class Microservice(Enum):
	Network = "network"
	Memory = "memory"
	Compute = "compute"
	Storage = "storage"
	
class NetworkMicroservice:
	functions = ["network_do_work"]

	def __init__(self, num_providers):
		self.num_providers = num_providers
		self.microservice_type = Microservice.Network
	
class MemoryMicroservice:
	functions = ["memory_do_work"]

	def __init__(self, num_providers):
		self.num_providers = num_providers
		self.microservice_type = Microservice.Memory

class ComputeMicroservice:
	functions = ["compute_do_work"]

	def __init__(self, num_providers):
		self.num_providers = num_providers
		self.microservice_type = Microservice.Compute

class StorageMicroservice:
	functions = ["storage_do_work"]

	def __init__(self, num_providers):
		self.num_providers = num_providers
		self.microservice_type = Microservice.Storage

class AccessPattern(Enum):
	Fixed = 1
	Dynamic = 2

class OperationTree:
	def __init__(self, microservice_function, first=None, second=None, third=None):
		self.microservice_function = microservice_function
		self.first = first
		self.second = second
		self.third = third

	def traverseTree(self, accessPattern=None, isFirstChild=True):
		requestStructure = ""
                if isFirstChild: 
			requestStructure += '{'
		else:
			requestStructure += ',{'
		
		requestStructure += '"val": ' + '"'+str(self.microservice_function) + '"'
		if accessPattern != None:
			requestStructure += ',' + '"accessPattern": ' + '"'+ str(accessPattern) + '"'

		if self.first != None or self.second != None or self.third != None:
			requestStructure += ',' + '"children": ['
			if self.first != None:
				requestStructure += self.first[0].traverseTree(self.first[1])
			if self.second != None:
				requestStructure += self.second[0].traverseTree(self.second[1], False)
			if self.third != None:
				requestStructure += self.third[0].traverseTree(self.third[1], False)
			requestStructure += ']'
		requestStructure += "}"
		return requestStructure
		


class OperationType:
	def __init__(self, name, tree):
		self.name = name
		self.opTree = tree

class Service:
	def __init__(self, name):
		self.microservices = list()
		self.name = name
		self.opTypes = set()
		self.legal_boilerplate = "/*\n  * (C) 2015 The University of Chicago\n  *\n  * See COPYRIGHT in top-level directory.\n*/\n"
	
	def addMicroservice(self, microservice):
		self.microservices.append(microservice)

	def addOperationType(self, op):
		self.opTypes.add(op)

	def __generateClientHeader(self):
		filename = self.name + str("_client.h")
		f = open(filename, "w")
		f.write(self.legal_boilerplate)
		op_enum_string = "enum " + self.name + "_optype {\n"

		for index, op in enumerate(self.opTypes):
			if index < (len(self.opTypes) - 1):
				op_enum_string += "\t" + op.name + ",\n"
			else:
				op_enum_string += "\t" + op.name + "\n"
		op_enum_string += "};\n\ntypedef enum " + self.name + "_optype " + self.name + "_optype;\n\n"

		f.write(op_enum_string)
		
		op_structure_getter_func = "const char* get_" + self.name + "_service_request_structure(" + self.name + "_optype op) {\n"
		op_structure_getter_func += "\tswitch(op) {\n"
		for op in self.opTypes:
			op_structure_getter_func += "\t case " + op.name + ":\n" + "\t  return \"" + str(op.opTree.traverseTree()) + "\";\n"
		op_structure_getter_func += "\t}\n}"
		f.write(op_structure_getter_func)
		f.flush()
		f.close()

	def __generateServiceHeader(self):
		filename = self.name + str("_server.h")
		f = open(filename, "w")
		f.write(self.legal_boilerplate)

		service_core_struct = ""
		for microservice in self.microservices:
			service_core_struct += "\tint " + microservice.microservice_type.value + "[" + str(microservice.num_providers) + "];\n"
		service_struct = "typedef struct {\n" + service_core_struct + "} " + self.name +"_service;\n\n"

		service_provider_num_constants = ""
		
		for microservice in self.microservices:
			service_provider_num_constants += "static int " + self.name + "_service_N_" + microservice.microservice_type.value + " = " + str(microservice.num_providers) + ";\n"
		service_clients_and_providers = ""
		for microservice in self.microservices:
			service_clients_and_providers += microservice.microservice_type.value + "_client_t " + self.name + "_" + microservice.microservice_type.value + "_clt;\n"
			service_clients_and_providers += microservice.microservice_type.value + "_provider_handle_t " + self.name + "_" + microservice.microservice_type.value + "_local_ph[" +str(microservice.num_providers) + "];\n"

		service_clients_and_providers += "\n\n"
		
		service_provider_num_constants += "\n\n"
	
		service_init_function = "void initialize_" + self.name + "_service(margo_instance_id mid, " + self.name + "_service* d) {\n"
		service_init_function += "  hg_addr_t my_address;\n  margo_addr_self(mid, &my_address);\n\n"
	
		for microservice in self.microservices:
			service_init_function += "  " + microservice.microservice_type.value + "_client_init(mid, &" + self.name + "_" + microservice.microservice_type.value + "_clt);\n"
			service_init_function += "  for(int i = 0; i < " + self.name + "_service_N_" + microservice.microservice_type.value + "; i++) {\n"
			service_init_function += "    d->" + microservice.microservice_type.value + "[i] = GENERATE_UNIQUE_PROVIDER_ID();\n"
			service_init_function += "    " + microservice.microservice_type.value + "_provider_register(mid, d->" + microservice.microservice_type.value + "[i], " + \
							microservice.microservice_type.value.upper() + "_ABT_POOL_DEFAULT, " + microservice.microservice_type.value.upper() + "_PROVIDER_IGNORE);\n"
			service_init_function += "    " + microservice.microservice_type.value + "_provider_handle_create(" + self.name + "_" + microservice.microservice_type.value + "_clt, my_address, d->" + microservice.microservice_type.value + "[i], &" + self.name + "_" + microservice.microservice_type.value + "_local_ph[i]);\n"
			service_init_function += "  }\n\n"

		service_init_function += "}\n\n"

		service_finalize_function = "void finalize_" + self.name + "_service(margo_instance_id mid, " + self.name + "_service* d) {\n"
		service_finalize_function += "  hg_addr_t my_address;\n  margo_addr_self(mid, &my_address);\n\n"

		for microservice in self.microservices:
			service_finalize_function += "  for(int i = 0; i < " + self.name + "_service_N_" + microservice.microservice_type.value + "; i++) {\n"
			service_finalize_function += "    " + microservice.microservice_type.value + "_provider_handle_release(" + self.name + "_" + microservice.microservice_type.value +"_local_ph[i]);\n"
			service_finalize_function += "  }\n"
			service_finalize_function += "  " + microservice.microservice_type.value + "_client_finalize(" + self.name + "_" + microservice.microservice_type.value + "_clt);\n\n"
			
		service_finalize_function += "}\n\n"	
		service_provider_handle_generation = ""

		for microservice in self.microservices:
			service_provider_handle_generation += microservice.microservice_type.value + "_provider_handle_t " + self.name + "_service_generate_" + microservice.microservice_type.value + "_provider_handle(enum AccessPattern p) {\n"
			service_provider_handle_generation += "  return " + self.name + "_" + microservice.microservice_type.value + "_local_ph[rand()%" + self.name + "_service_N_" + microservice.microservice_type.value + "];\n"
			service_provider_handle_generation += "}\n\n"

		f.write(service_struct)
		f.write(service_provider_num_constants)
		f.write(service_clients_and_providers)
		f.write(service_init_function)
		f.write(service_finalize_function)
		f.write(service_provider_handle_generation)
		f.flush()
		f.close()

	def generateHeaders(self):
		self.__generateClientHeader()
		self.__generateServiceHeader()


class MochiExperiment:
	def __init__(self, name):
		self.name = name
		self.legal_boilerplate = "/*\n  * (C) 2015 The University of Chicago\n  *\n  * See COPYRIGHT in top-level directory.\n*/\n"
		self.services = list()

	def addService(self, s):
		self.services.append(s)

	def generateHeaders(self):
		filename = "user_services.h"
		f = open(filename, "w")
		f.write(self.legal_boilerplate)
		f.write("#ifndef USER_SERVICES_H\n#define USER_SERVICES_H\n\n")
		f.write("#include \"microservices/compute/compute-client.h\"\n")
		f.write("#include \"microservices/memory/memory-client.h\"\n")
		f.write("#include \"microservices/network/network-client.h\"\n")
		f.write("#include \"microservices/storage/storage-client.h\"\n\n")

		for service in self.services:
			f.write("/* " + service.name + " service definitions */ \n")
			service_provider_handle_definitions = ""
			for microservice in service.microservices:
				service_provider_handle_definitions += "extern " + microservice.microservice_type.value + "_provider_handle_t " + service.name + "_service_generate_" + microservice.microservice_type.value + "_provider_handle(enum AccessPattern p);\n"
			service_provider_handle_definitions += "\n\n"
			f.write(service_provider_handle_definitions)
			service.generateHeaders()

		f.write("#endif")
		f.flush()
		f.close()
	
def main():
	a = OperationTree(NetworkMicroservice.functions[0])
	b = OperationTree(ComputeMicroservice.functions[0], (a, AccessPattern.Fixed), (a, AccessPattern.Dynamic))
	c = OperationTree(StorageMicroservice.functions[0], (b, AccessPattern.Dynamic))

	op1 = OperationType("op1", c)
	op2 = OperationType("op2", b)
	s = Service("dummy")
	s.addMicroservice(NetworkMicroservice(2))
	s.addMicroservice(ComputeMicroservice(1))
	s.addMicroservice(StorageMicroservice(1))
	s.addMicroservice(MemoryMicroservice(1))
	s.addOperationType(op1)
	s.addOperationType(op2)
	m = MochiExperiment("test")
	m.addService(s)
	m.generateHeaders()

main()

