#ifndef __MEMORY_SERVER_H
#define __MEMORY_SERVER_H

#include <margo.h>
#include "../common.h"
#include "memory-common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEMORY_ABT_POOL_DEFAULT ABT_POOL_NULL

typedef struct memory_provider* memory_provider_t;
#define MEMORY_PROVIDER_NULL ((memory_provider_t)NULL)
#define MEMORY_PROVIDER_IGNORE ((memory_provider_t*)NULL)

/**
 *  * @brief Creates a new MEMORY provider. If MEMORY_PROVIDER_IGNORE
 *   * is passed as last argument, the provider will be automatically
 *    * destroyed when calling :code:`margo_finalize`.
 *     *
 *      * @param[in] mid Margo instance
 *       * @param[in] provider_id provider id
 *        * @param[in] pool Argobots pool
 *         * @param[out] provider provider handle
 *          *
 *           * @return MEMORY_SUCCESS or error code defined in memory-common.h
 *            */
int memory_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        memory_provider_t* provider);

/**
 *  * @brief Destroys the memory provider and deregisters its RPC.
 *   *
 *    * @param[in] provider memory provider
 *     *
 *      * @return MEMORY_SUCCESS or error code defined in memory-common.h
 *       */
int memory_provider_destroy(
        memory_provider_t provider);

#ifdef __cplusplus
}
#endif

#endif

