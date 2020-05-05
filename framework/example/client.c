#include "client_helper.h"
#include "dummy_client.h"

int main(int argc, char **argv) {

   INIT_MARGO(ofi+verbs, 0);

   INIT_CLIENT(dummy);

   /* Generate a workload */
   Workload w;

   dummy_optype op[100];
   int N = 100;
   int rate[100];
   int workload_factor[100];
   AccessPattern accessPattern;

   for(int i = 0; i < N; i++) {
     if(i < 50) {
       op[i] = op1;
       rate[i] = DEFAULT_REQUEST_RATE;
       workload_factor[i] = 1;
     } else {
       op[i] = op2;
       rate[i] = 2*DEFAULT_REQUEST_RATE;
       workload_factor[i] = 2;
     }
   }

   accessPattern = Fixed; 

   GENERATE_WORKLOAD(dummy, op, workload_factor, rate, N, accessPattern, w);

   RUN_WORKLOAD(dummy, w);
   
   CLEANUP_WORKLOAD(w);

   FINALIZE_CLIENT(dummy);

   FINALIZE_MARGO(1);
}
