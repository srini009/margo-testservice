#ifndef __STORAGE_SERVER_H
#define __STORAGE_SERVER_H

#include <margo.h>
#include "storage-common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STORAGE_ABT_POOL_DEFAULT ABT_POOL_NULL

typedef struct storage_provider* storage_provider_t;
#define STORAGE_PROVIDER_NULL ((storage_provider_t)NULL)
#define STORAGE_PROVIDER_IGNORE ((storage_provider_t*)NULL)

/**
 *  * @brief Creates a new STORAGE provider. If STORAGE_PROVIDER_IGNORE
 *   * is passed as last argument, the provider will be automatically
 *    * destroyed when calling :code:`margo_finalize`.
 *     *
 *      * @param[in] mid Margo instance
 *       * @param[in] provider_id provider id
 *        * @param[in] pool Argobots pool
 *         * @param[out] provider provider handle
 *          *
 *           * @return STORAGE_SUCCESS or error code defined in storage-common.h
 *            */
int storage_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        storage_provider_t* provider);

/**
 *  * @brief Destroys the storage provider and deregisters its RPC.
 *   *
 *    * @param[in] provider storage provider
 *     *
 *      * @return STORAGE_SUCCESS or error code defined in storage-common.h
 *       */
int storage_provider_destroy(
        storage_provider_t provider);

/**
 *  * @brief Creates handles and connections to downstream microservice dependencies
 *   *
 *    * @param[in] mid Margo instance
 *     *  @param[in] p provider id
 *      *  @param[in] svr_addr server address
 *       *
 *        * @return STORAGE_SUCCESS or error code defined in storage-common.h
 *         */
void storage_create_downstream_handles(
	margo_instance_id mid, 
	uint16_t p, 
	hg_addr_t svr_addr);
#ifdef __cplusplus
}
#endif

#endif

