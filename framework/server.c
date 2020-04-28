#include "include/service_macros.h"
#include "dummy_server.h"

int main(int argc, char** argv)
{
    INIT_MARGO(ofi+verbs, 2);

    dummy_service d;

    INIT_AND_RUN_SERVICE(dummy, &d);
    
    FINALIZE_SERVICE(dummy, &d);

    FINALIZE_MARGO(1);

    return 0;
}
