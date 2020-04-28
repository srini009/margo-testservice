#ifndef COMMON_H
#define COMMON_H

#include "../include/types.h"
#include "../include/defaults.h"
#include "../user_services.h"

#define GENERATE_PROVIDER_HANDLE(serviceName, microservice, accessPattern) \
  serviceName##_service_generate_##microservice##_provider_handle(accessPattern)

#endif
