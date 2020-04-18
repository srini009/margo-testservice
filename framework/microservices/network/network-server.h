#ifndef __NETWORK_SERVER_H
#define __NETWORK_SERVER_H

#include <margo.h>
#include "network-common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NETWORK_ABT_POOL_DEFAULT ABT_POOL_NULL

typedef struct network_provider* network_provider_t;
#define NETWORK_PROVIDER_NULL ((network_provider_t)NULL)
#define NETWORK_PROVIDER_IGNORE ((network_provider_t*)NULL)

/**
 *  * @brief Creates a new NETWORK provider. If NETWORK_PROVIDER_IGNORE
 *   * is passed as last argument, the provider will be automatically
 *    * destroyed when calling :code:`margo_finalize`.
 *     *
 *      * @param[in] mid Margo instance
 *       * @param[in] provider_id provider id
 *        * @param[in] pool Argobots pool
 *         * @param[out] provider provider handle
 *          *
 *           * @return NETWORK_SUCCESS or error code defined in network-common.h
 *            */
int network_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        network_provider_t* provider);

/**
 *  * @brief Destroys the Network provider and deregisters its RPC.
 *   *
 *    * @param[in] provider Network provider
 *     *
 *      * @return NETWORK_SUCCESS or error code defined in network-common.h
 *       */
int network_provider_destroy(
        network_provider_t provider);

/**
 *  * @brief Creates handles and connections to downstream microservice dependencies
 *   *
 *    * @param[in] mid Margo instance
 *     *  @param[in] p provider id
 *      *  @param[in] svr_addr server address
 *       *
 *        * @return NETWORK_SUCCESS or error code defined in network-common.h
 *         */
void network_create_downstream_handles(
	margo_instance_id mid, 
	uint16_t p, 
	hg_addr_t svr_addr);

#ifdef __cplusplus
}
#endif

#endif

