#ifndef COMMON_H
#define COMMON_H

#include "../include/types.h"
#include "../include/defaults.h"
#include "../user_services.h"
#include "../include/jsmn.h"
#include <assert.h>

void substring(char s[], char sub[], int p, int l) {
   int c = 0;
   
   while (c < l) {
      sub[c] = s[p+c-1];
      c++;
   }
   sub[c] = '\0';
}

int num_children(jsmntok_t *t, int num, int *index) {

  *index = -1;
  int n = 0;
  for(int i = 0; i < num; i++) {
    if(t[i].type == JSMN_ARRAY) {
      n = t[i].size;
      *index = i;
      break;  
    }
  } 

  return n;
}

void extract_next_link_info(jsmntok_t *t, int array_index, int *end_last_child, int length, char *request, char *subrequest, int *service_id, int *microservice_id, int *accessPattern) {

  /*Extract endpoints of the nth child*/
  char fname[50], ap[50], sname[50];
  for(int i = array_index; i < length; i++) {
    if(t[i].type == JSMN_OBJECT && t[i].start == (*end_last_child)+1) {
      *end_last_child = t[i].end;
      substring(request, subrequest, t[i].start+1, (t[i].end-t[i].start));

      assert(t[i+2].type == JSMN_PRIMITIVE);
      substring(request, sname, t[i+2].start+1, 1);
      *service_id = atoi(sname);

      assert(t[i+4].type == JSMN_PRIMITIVE);
      substring(request, fname, t[i+4].start+1, 1);
      *microservice_id = atoi(fname);

      assert(t[i+6].type == JSMN_PRIMITIVE);
      substring(request, ap, t[i+6].start+1, 1);
      *accessPattern = atoi(ap);
      break;
    }
  }
}

#define GENERATE_DOWNSTREAM_REQUESTS(request_structure, workload_factor, bulk_handle, partial_result)\
  int r; \
  jsmn_parser p; \
  jsmntok_t t[128]; /* We expect no more than 128 JSON tokens */ \
  jsmn_init(&p); \
  r = jsmn_parse(&p, request_structure, strlen(request_structure), t, 128); \
  int index; \
  int n_children = num_children(t, r, &index); \
  int end_last_child = t[index].start; \
  for(int child = 0; child < n_children; child++) { \
     char subrequest[2000]; \
     int microservice_id, accessPattern, service_id; \
     extract_next_link_info(t, index, &end_last_child, r, request_structure, subrequest, &service_id, &microservice_id, &accessPattern); \
     generate_request(service_id, microservice_id, accessPattern, workload_factor, bulk_handle, subrequest, partial_result); \
  } \


#endif
