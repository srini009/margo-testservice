#include "client_helper.h"
#include "dummy_client.h"
#include "metoo_client.h"

int main(int argc, char **argv) {

   INIT_MARGO(ofi+verbs, 0);

   INIT_CLIENT(dummy);
   INIT_CLIENT(metoo);

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
   /* Generate a workload */
   Workload y;

   dummy_optype op_[100];
   int N_ = 100;
   int rate_[100];
   int workload_factor_[100];
   AccessPattern accessPattern_;

   for(int i = 0; i < N; i++) {
       op_[i] = op3;
       rate_[i] = DEFAULT_REQUEST_RATE;
       workload_factor_[i] = 1;
   }

   accessPattern_ = Dynamic; 

   GENERATE_WORKLOAD(metoo, op_, workload_factor_, rate_, N_, accessPattern_, y);

   RUN_WORKLOAD(metoo, y);
   
   CLEANUP_WORKLOAD(y);

   FINALIZE_CLIENT(dummy);
   FINALIZE_CLIENT(metoo);

   FINALIZE_MARGO(1);
}
