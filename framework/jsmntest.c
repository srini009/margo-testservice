#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "include/jsmn.h"

static char js[] = "{\"service_id\": 0, \"microservice_function\": 2,\"children\": [{\"service_id\": 0, \"microservice_function\": 0,\"accessPattern\": 0},{\"service_id\": 0, \"microservice_function\": 0,\"accessPattern\": 1}]}";

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
      fprintf(stderr, "A child is at %d, %d\n", t[i].start, t[i].end);
      *end_last_child = t[i].end;
      substring(request, subrequest, t[i].start+1, (t[i].end-t[i].start));
      fprintf(stderr, "Subrequest is %s\n", subrequest);
      fprintf(stderr, "Accesspattern is %d\n", *accessPattern); 
      assert(t[i+2].type == JSMN_PRIMITIVE);
      substring(request, sname, t[i+2].start+1, 1);
      *service_id = atoi(sname);

      assert(t[i+4].type == JSMN_PRIMITIVE);
      substring(request, fname, t[i+4].start+1, 1);
      *microservice_id = atoi(fname);

      assert(t[i+6].type == JSMN_PRIMITIVE);
      substring(request, ap, t[i+6].start+1, 1);
      *accessPattern = atoi(ap);

      fprintf(stderr, "Service id: %d, Microservice_function: %d, Accesspattern: %d\n", *service_id, *microservice_id, *accessPattern); 
    }
  }
}

int main() {
  fprintf(stderr, "%s\n", js);

  int r;
  jsmn_parser p;
  jsmntok_t t[128]; /* We expect no more than 128 JSON tokens */
  jsmn_init(&p);
  r = jsmn_parse(&p, js, strlen(js), t, 128);
  
  for(int i = 0; i < r; i++)
    fprintf(stderr, "Type: %d, Start: %d, End: %d, Num_nested: %d\n", t[i].type, t[i].start, t[i].end, t[i].size);

  fprintf(stderr, "Number of tokens: %d\n", r);

  int index;
  int n_children = num_children(t, r, &index);
  int end_last_child = t[index].start;
  for(int child = 0; child < n_children; child++) {
     char subrequest[2000];
     int microservice_function, accessPattern, service_id;
     fprintf(stderr, "End last child is %d\n", end_last_child);
     extract_next_link_info(t, index, &end_last_child, r, js, subrequest, &service_id, &microservice_function, &accessPattern);
     //Call function
     switch(microservice_function) {
          case 0:
          break;
	  case 1:
          break;
          case 2:
          break;
	  case 3:
          break;
     }
  }
  
  return 0;
}
