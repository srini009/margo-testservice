#!/usr/bin/env python
# (C) 2015 The University of Chicago
#
# See COPYRIGHT in top-level directory.

from enum import Enum

#HACK ALERT
#Make sure this matches the generate_request switch case
microservice_function_ids = {"network_do_work": 0, "memory_do_work": 1, "compute_do_work": 2, "storage_do_work": 3}

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
	Fixed = 0
	Dynamic = 1

class OperationTree:
	def __init__(self, microservice_function, first=None, second=None, third=None):
		self.microservice_function = microservice_function
		self.first = first
		self.second = second
		self.third = third

	def traverseTree(self, id_, accessPattern=None, isFirstChild=True):
		requestStructure = ""
                if isFirstChild: 
			requestStructure += '{'
		else:
			requestStructure += ',{'
		
		requestStructure += '\\"service_id\\": ' + str(id_) + ', \\"microservice_function\\": ' + str(microservice_function_ids[str(self.microservice_function)]) 
		if accessPattern != None:
			requestStructure += ',' + '\\"accessPattern\\": ' + str(accessPattern.value)

		if self.first != None or self.second != None or self.third != None:
			requestStructure += ',' + '\\"children\\": ['
			if self.first != None:
				requestStructure += self.first[0].traverseTree(id_, self.first[1])
			if self.second != None:
				requestStructure += self.second[0].traverseTree(id_, self.second[1], False)
			if self.third != None:
				requestStructure += self.third[0].traverseTree(id_, self.third[1], False)
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

	def __generateClientHeader(self, id_):
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
		
		op_structure_getter_func = "char* get_" + self.name + "_service_request_structure(" + self.name + "_optype op) {\n"
		op_structure_getter_func += "\tswitch(op) {\n"
		for op in self.opTypes:
			op_structure_getter_func += "\t case " + op.name + ":\n" + "\t  return \"" + str(op.opTree.traverseTree(id_)) + "\";\n"
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
		service_provider_num_constants += "static int " + self.name + "_num_remote_servers;\n"

		service_clients_and_providers = ""
		for microservice in self.microservices:
			service_clients_and_providers += microservice.microservice_type.value + "_client_t " + self.name + "_" + microservice.microservice_type.value + "_clt;\n"
			service_clients_and_providers += microservice.microservice_type.value + "_provider_handle_t " + self.name + "_" + microservice.microservice_type.value + "_local_ph[" +str(microservice.num_providers) + "];\n"
			service_clients_and_providers += microservice.microservice_type.value + "_provider_handle_t *" + self.name + "_" + microservice.microservice_type.value + "_remote_ph;\n"

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
			service_finalize_function += "  for(int i = 0; i < " + self.name + "_service_N_" + microservice.microservice_type.value + "*" + self.name + "_num_remote_servers; i++) {\n"
			service_finalize_function += "    " + microservice.microservice_type.value + "_provider_handle_release(" + self.name + "_" + microservice.microservice_type.value +"_remote_ph[i]);\n"
			service_finalize_function += "  }\n"
			service_finalize_function += "  " + microservice.microservice_type.value + "_client_finalize(" + self.name + "_" + microservice.microservice_type.value + "_clt);\n\n"
			
		service_finalize_function += "}\n\n"	
		service_provider_handle_generation = ""

		for microservice in self.microservices:
			service_provider_handle_generation += microservice.microservice_type.value + "_provider_handle_t " + self.name + "_service_generate_" + microservice.microservice_type.value + "_provider_handle(enum AccessPattern p) {\n"
			service_provider_handle_generation += "  switch(p) {\n"
			service_provider_handle_generation += "    case(Fixed): \n"
			service_provider_handle_generation += "      return " + self.name + "_" + microservice.microservice_type.value + "_local_ph[rand()%" + self.name + "_service_N_" + microservice.microservice_type.value + "];\n"
			service_provider_handle_generation += "    case(Dynamic): \n"
			service_provider_handle_generation += "      return " + self.name + "_" + microservice.microservice_type.value + "_remote_ph[rand()%" + self.name + "_service_N_" + microservice.microservice_type.value + "*" + self.name + "_num_remote_servers];\n"
			service_provider_handle_generation += "  }\n"
			service_provider_handle_generation += "}\n\n"

		service_write_local_provider_ids = ""
		service_write_local_provider_ids += "void " + self.name + "_write_local_provider_ids(int my_id, " + self.name + "_service* d) {\n"
		service_write_local_provider_ids += "  char filename[100];\n  sprintf(filename, \"" + self.name + "_provider_ids_%d.txt\", my_id);\n"
		service_write_local_provider_ids += "  FILE *fp = fopen(filename, \"w\");\n\n\n"
		for microservice in self.microservices:
			if(microservice.microservice_type.value == "network"):
				service_write_local_provider_ids += "  fprintf(fp, \"0 %d\\n\", " + self.name + "_service_N_" + microservice.microservice_type.value + ");\n"
			if(microservice.microservice_type.value == "memory"):
				service_write_local_provider_ids += "  fprintf(fp, \"1 %d\\n\", " + self.name + "_service_N_" + microservice.microservice_type.value + ");\n"
			if(microservice.microservice_type.value == "compute"):
				service_write_local_provider_ids += "  fprintf(fp, \"2 %d\\n\", " + self.name + "_service_N_" + microservice.microservice_type.value + ");\n"
			if(microservice.microservice_type.value == "storage"):
				service_write_local_provider_ids += "  fprintf(fp, \"3 %d\\n\", " + self.name + "_service_N_" + microservice.microservice_type.value + ");\n"

			service_write_local_provider_ids += "  for(int i = 0; i < " + self.name + "_service_N_" + microservice.microservice_type.value + "; i++)\n"
			service_write_local_provider_ids += "    fprintf(fp, \"%d\\n\", d->" + microservice.microservice_type.value + "[i]);\n"

		service_write_local_provider_ids += "  fflush(fp); \n  fclose(fp);\n"
		service_write_local_provider_ids += "}\n\n"

		service_init_remote_provider_handles = ""
		service_init_remote_provider_handles += "void " + self.name + "_initialize_remote_provider_handles(margo_instance_id mid, int my_id, int total_servers) {\n"
		service_init_remote_provider_handles += "  " + self.name + "_num_remote_servers = total_servers - 1;\n"
		for microservice in self.microservices:
			service_init_remote_provider_handles += "  int n_" + microservice.microservice_type.value + " = " + self.name + "_service_N_" + microservice.microservice_type.value + "*" + self.name + "_num_remote_servers;\n"
			service_init_remote_provider_handles += "  " + self.name + "_" + microservice.microservice_type.value + "_remote_ph = (" + microservice.microservice_type.value + "_provider_handle_t*)malloc(n_" + microservice.microservice_type.value + "*sizeof(" + microservice.microservice_type.value + "_provider_handle_t));\n"

		service_init_remote_provider_handles += "\n"

		service_init_remote_provider_handles += "  int j, k = 0;\n"
		service_init_remote_provider_handles += "  for(int i=0; i < total_servers; i++) {\n"
		service_init_remote_provider_handles += "    if(i != my_id) {\n"
		service_init_remote_provider_handles += "      char filename[100], filename1[100];\n      char str[1000], svr_addr_str[80];\n"
		service_init_remote_provider_handles += "      sprintf(filename, \"" + self.name + "_provider_ids_%d.txt\", i);\n"
		service_init_remote_provider_handles += "      FILE *fp = fopen(filename, \"r\");\n      FILE * fp1;\n      int ret;\n"
		service_init_remote_provider_handles += "      sprintf(filename1, \"server_addr_%d.txt\", i);\n\n"
		service_init_remote_provider_handles += "      fp1 = fopen(filename1, \"r\");\n"
		service_init_remote_provider_handles += "      fscanf(fp1, \"%s\", svr_addr_str);\n      hg_addr_t remote_address;\n      margo_addr_lookup(mid, svr_addr_str, &remote_address);\n      fclose(fp1);\n"

		for microservice in self.microservices:
			service_init_remote_provider_handles += "      fgets(str, 60, fp);\n      j = 0;\n"
			service_init_remote_provider_handles += "      while(j < " + self.name + "_service_N_" + microservice.microservice_type.value + ") {\n"
			service_init_remote_provider_handles += "        int id;\n        fgets(str, 60, fp);\n        sscanf(str, \"%d\", &id);\n"
			service_init_remote_provider_handles += "        ret = " + microservice.microservice_type.value + "_provider_handle_create(" + self.name + "_" + microservice.microservice_type.value + "_clt, remote_address, id, &" + self.name + "_" + microservice.microservice_type.value +"_remote_ph[j+(k*" + self.name + "_service_N_" + microservice.microservice_type.value + ")]);\n"
			service_init_remote_provider_handles += "        assert(ret == " + microservice.microservice_type.value.upper() + "_SUCCESS);\n"
			service_init_remote_provider_handles += "        j++;\n"
			service_init_remote_provider_handles += "      }\n" 		
		service_init_remote_provider_handles += "    }\n"
		service_init_remote_provider_handles += "  }\n"
		service_init_remote_provider_handles += "}\n\n"
		f.write(service_struct)
		f.write(service_provider_num_constants)
		f.write(service_clients_and_providers)
		f.write(service_init_function)
		f.write(service_finalize_function)
		f.write(service_provider_handle_generation)
		f.write(service_write_local_provider_ids)
		f.write(service_init_remote_provider_handles)
		f.flush()
		f.close()

	def generateHeaders(self, id_):
		self.__generateClientHeader(id_)
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

		for service_id, service in enumerate(self.services):
			f.write("/* " + service.name + " service definitions */ \n")
			service_provider_handle_definitions = ""
			for microservice in service.microservices:
				service_provider_handle_definitions += "extern " + microservice.microservice_type.value + "_provider_handle_t " + service.name + "_service_generate_" + microservice.microservice_type.value + "_provider_handle(enum AccessPattern p);\n"
			service_provider_handle_definitions += "\n\n"
			f.write(service_provider_handle_definitions)
			service.generateHeaders(service_id)

		service_generate_request = "void generate_request(int service_id, int microservice_id, enum AccessPattern p, int workload_factor, hg_bulk_t bulk, hg_string_t request_structure, int32_t *partial_result) {\n"
		service_generate_request += "   switch(service_id) {\n"
		for service_id, service in enumerate(self.services):
			service_generate_request += "     case(" + str(service_id) + "):\n"
			service_generate_request += "       switch(microservice_id) {\n"
			service_generate_request += "         case(0):\n"
			service_generate_request += "           network_do_work(" + service.name + "_service_generate_network_provider_handle(p), workload_factor, bulk, request_structure, partial_result);\n"
			service_generate_request += "           break;\n"
			service_generate_request += "         case(1):\n"
			service_generate_request += "           compute_do_work(" + service.name + "_service_generate_compute_provider_handle(p), workload_factor, bulk, request_structure, partial_result);\n"
			service_generate_request += "           break;\n"
			service_generate_request += "         case(2):\n"
			service_generate_request += "           memory_do_work(" + service.name + "_service_generate_memory_provider_handle(p), workload_factor, bulk, request_structure, partial_result);\n"
			service_generate_request += "           break;\n"
			service_generate_request += "         case(3):\n"
			service_generate_request += "           storage_do_work(" + service.name + "_service_generate_storage_provider_handle(p), workload_factor, bulk, request_structure, partial_result);\n"
			service_generate_request += "           break;\n"
			service_generate_request += "       }\n"
			service_generate_request += "       break;\n"
			service_generate_request += "  }\n"	
		service_generate_request += "}\n\n"
		f.write(service_generate_request) 
				
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

	#Generate test data
	a_ = OperationTree(ComputeMicroservice.functions[0])
	b_ = OperationTree(NetworkMicroservice.functions[0], (a_, AccessPattern.Fixed))
	s_ = Service("dummy")
	s_.addMicroservice(NetworkMicroservice(1))
	s_.addMicroservice(ComputeMicroservice(1))
	s_.addMicroservice(StorageMicroservice(1))
	s_.addMicroservice(MemoryMicroservice(1))
	op = OperationType("testing", b_)
	s_.addOperationType(op)
	m = MochiExperiment("test")
	m.addService(s_)
	m.generateHeaders()

main()

