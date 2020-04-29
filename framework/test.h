/*
  * (C) 2015 The University of Chicago
  *
  * See COPYRIGHT in top-level directory.
*/
typedef struct {
	int network[2];
	int compute[1];
	int storage[1];
	int memory[1];
} dummy_service;

static int dummy_service_N_network = 2;
static int dummy_service_N_compute = 1;
static int dummy_service_N_storage = 1;
static int dummy_service_N_memory = 1;
static int dummy_num_remote_servers;


network_client_t dummy_network_clt;
network_provider_handle_t dummy_network_local_ph[2];
network_provider_handle_t *dummy_network_remote_ph;
compute_client_t dummy_compute_clt;
compute_provider_handle_t dummy_compute_local_ph[1];
compute_provider_handle_t *dummy_compute_remote_ph;
storage_client_t dummy_storage_clt;
storage_provider_handle_t dummy_storage_local_ph[1];
storage_provider_handle_t *dummy_storage_remote_ph;
memory_client_t dummy_memory_clt;
memory_provider_handle_t dummy_memory_local_ph[1];
memory_provider_handle_t *dummy_memory_remote_ph;


void initialize_dummy_service(margo_instance_id mid, dummy_service* d) {
  hg_addr_t my_address;
  margo_addr_self(mid, &my_address);

  network_client_init(mid, &dummy_network_clt);
  for(int i = 0; i < dummy_service_N_network; i++) {
    d->network[i] = GENERATE_UNIQUE_PROVIDER_ID();
    network_provider_register(mid, d->network[i], NETWORK_ABT_POOL_DEFAULT, NETWORK_PROVIDER_IGNORE);
    network_provider_handle_create(dummy_network_clt, my_address, d->network[i], &dummy_network_local_ph[i]);
  }

  compute_client_init(mid, &dummy_compute_clt);
  for(int i = 0; i < dummy_service_N_compute; i++) {
    d->compute[i] = GENERATE_UNIQUE_PROVIDER_ID();
    compute_provider_register(mid, d->compute[i], COMPUTE_ABT_POOL_DEFAULT, COMPUTE_PROVIDER_IGNORE);
    compute_provider_handle_create(dummy_compute_clt, my_address, d->compute[i], &dummy_compute_local_ph[i]);
  }

  storage_client_init(mid, &dummy_storage_clt);
  for(int i = 0; i < dummy_service_N_storage; i++) {
    d->storage[i] = GENERATE_UNIQUE_PROVIDER_ID();
    storage_provider_register(mid, d->storage[i], STORAGE_ABT_POOL_DEFAULT, STORAGE_PROVIDER_IGNORE);
    storage_provider_handle_create(dummy_storage_clt, my_address, d->storage[i], &dummy_storage_local_ph[i]);
  }

  memory_client_init(mid, &dummy_memory_clt);
  for(int i = 0; i < dummy_service_N_memory; i++) {
    d->memory[i] = GENERATE_UNIQUE_PROVIDER_ID();
    memory_provider_register(mid, d->memory[i], MEMORY_ABT_POOL_DEFAULT, MEMORY_PROVIDER_IGNORE);
    memory_provider_handle_create(dummy_memory_clt, my_address, d->memory[i], &dummy_memory_local_ph[i]);
  }

}

void finalize_dummy_service(margo_instance_id mid, dummy_service* d) {
  hg_addr_t my_address;
  margo_addr_self(mid, &my_address);

  for(int i = 0; i < dummy_service_N_network; i++) {
    network_provider_handle_release(dummy_network_local_ph[i]);
  }
  network_client_finalize(dummy_network_clt);

  for(int i = 0; i < dummy_service_N_compute; i++) {
    compute_provider_handle_release(dummy_compute_local_ph[i]);
  }
  compute_client_finalize(dummy_compute_clt);

  for(int i = 0; i < dummy_service_N_storage; i++) {
    storage_provider_handle_release(dummy_storage_local_ph[i]);
  }
  storage_client_finalize(dummy_storage_clt);

  for(int i = 0; i < dummy_service_N_memory; i++) {
    memory_provider_handle_release(dummy_memory_local_ph[i]);
  }
  memory_client_finalize(dummy_memory_clt);

}

network_provider_handle_t dummy_service_generate_network_provider_handle(enum AccessPattern p) {
  switch (p) {
    case Fixed:
     return dummy_network_local_ph[rand()%dummy_service_N_network];
     break;
    case Dynamic:
     fprintf(stderr, "In the game.\n");
     return dummy_network_remote_ph[rand()%(dummy_service_N_network*dummy_num_remote_servers)];
     break;
  }
}

compute_provider_handle_t dummy_service_generate_compute_provider_handle(enum AccessPattern p) {
  switch (p) {
    case Fixed:
     return dummy_compute_local_ph[rand()%dummy_service_N_compute];
     break;
    case Dynamic:
     fprintf(stderr, "In the game.\n");
     return dummy_compute_remote_ph[rand()%(dummy_service_N_compute*dummy_num_remote_servers)];
     break;
  }
}

storage_provider_handle_t dummy_service_generate_storage_provider_handle(enum AccessPattern p) {
  switch (p) {
    case Fixed:
     return dummy_storage_local_ph[rand()%dummy_service_N_storage];
     break;
    case Dynamic:
     fprintf(stderr, "In the game.\n");
     return dummy_storage_remote_ph[rand()%(dummy_service_N_storage*dummy_num_remote_servers)];
     break;
  }
}

memory_provider_handle_t dummy_service_generate_memory_provider_handle(enum AccessPattern p) {
  switch (p) {
    case Fixed:
     return dummy_memory_local_ph[rand()%dummy_service_N_memory];
     break;
    case Dynamic:
     fprintf(stderr, "In the game.\n");
     return dummy_memory_remote_ph[rand()%(dummy_service_N_memory*dummy_num_remote_servers)];
     break;
  }
}

void dummy_write_local_provider_ids(int my_id, dummy_service* d) {
  char filename[100];
  sprintf(filename, "dummy_provider_ids_%d.txt", my_id);
  FILE *fp = fopen(filename, "w");


  fprintf(fp, "0 %d\n", dummy_service_N_network);
  for(int i = 0; i < dummy_service_N_network; i++)
    fprintf(fp, "%d\n", d->network[i]);
  fprintf(fp, "2 %d\n", dummy_service_N_compute);
  for(int i = 0; i < dummy_service_N_compute; i++)
    fprintf(fp, "%d\n", d->compute[i]);
  fprintf(fp, "3 %d\n", dummy_service_N_storage);
  for(int i = 0; i < dummy_service_N_storage; i++)
    fprintf(fp, "%d\n", d->storage[i]);
  fprintf(fp, "1 %d\n", dummy_service_N_memory);
  for(int i = 0; i < dummy_service_N_memory; i++)
    fprintf(fp, "%d\n", d->memory[i]);
  fflush(fp); 
  fclose(fp);
}

void dummy_initialize_remote_provider_handles(margo_instance_id mid, int my_id, int total_servers) {

  dummy_num_remote_servers = total_servers - 1;

  int n_network = dummy_service_N_network*(total_servers - 1);
  dummy_network_remote_ph = (network_provider_handle_t*)malloc(n_network*sizeof(network_provider_handle_t));
  int n_compute = dummy_service_N_compute*(total_servers - 1);
  dummy_compute_remote_ph = (compute_provider_handle_t*)malloc(n_compute*sizeof(compute_provider_handle_t));
  int n_memory = dummy_service_N_memory*(total_servers - 1);
  dummy_memory_remote_ph = (memory_provider_handle_t*)malloc(n_memory*sizeof(memory_provider_handle_t));
  int n_storage = dummy_service_N_storage*(total_servers - 1);
  dummy_storage_remote_ph = (storage_provider_handle_t*)malloc(n_storage*sizeof(storage_provider_handle_t));

  int j;
 
  for(int i=0; i < total_servers; i++) {
    if(i != my_id) {
      char filename[100], filename1[100];
      char str[1000], svr_addr_str[80];
      sprintf(filename, "dummy_provider_ids_%d.txt", i);
      FILE *fp = fopen(filename, "r");
      FILE * fp1;
      sprintf(filename1, "server_addr_%d.txt", i);

      fp1 = fopen(filename1, "r");
      fscanf(fp1, "%s", svr_addr_str);
      hg_addr_t remote_address;
      margo_addr_lookup(mid, svr_addr_str, &remote_address);

      fgets(str, 60, fp);
      j = 0;
      while(j < dummy_service_N_network) {
        int id;
        fgets(str, 60, fp);
        sscanf(str, "%d", &id);
        network_provider_handle_create(dummy_network_clt, remote_address, id, &dummy_network_remote_ph[j]); 
        fprintf(stderr, "%p\n", dummy_network_remote_ph[j]);

        j++;
      }      

      fgets(str, 60, fp);
      j = 0;
      while(j < dummy_service_N_compute) {
        int id;
        fgets(str, 60, fp);
        sscanf(str, "%d", &id);
        compute_provider_handle_create(dummy_compute_clt, remote_address, id, &dummy_compute_remote_ph[j]); 
        j++;
      }      

      fgets(str, 60, fp);
      j = 0;
      while(j < dummy_service_N_memory) {
        int id;
        fgets(str, 60, fp);
        sscanf(str, "%d", &id);
        memory_provider_handle_create(dummy_memory_clt, remote_address, id, &dummy_memory_remote_ph[j]); 
        j++;
      }      

      fgets(str, 60, fp);
      j = 0;
      while(j < dummy_service_N_storage) {
        int id;
        fgets(str, 60, fp);
        sscanf(str, "%d", &id);
        storage_provider_handle_create(dummy_storage_clt, remote_address, id, &dummy_storage_remote_ph[j]); 
        j++;
      }      
    }
  } 

}

