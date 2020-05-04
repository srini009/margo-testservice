#include "include/client_helper.h"
#include "dummy_client.h"

int main(int argc, char **argv) {

   INIT_MARGO(ofi+verbs, 0);
   FINALIZE_MARGO(1);
}
