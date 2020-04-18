#ifndef __COMPUTE_SERVER_H
#define __COMPUTE_SERVER_H

#include <margo.h>
#include "compute-common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COMPUTE_ABT_POOL_DEFAULT ABT_POOL_NULL

typedef struct compute_provider* compute_provider_t;
#define COMPUTE_PROVIDER_NULL ((compute_provider_t)NULL)
#define COMPUTE_PROVIDER_IGNORE ((compute_provider_t*)NULL)

/**
 *  * @brief Creates a new COMPUTE provider. If COMPUTE_PROVIDER_IGNORE
 *   * is passed as last argument, the provider will be automatically
 *    * destroyed when calling :code:`margo_finalize`.
 *     *
 *      * @param[in] mid Margo instance
 *       * @param[in] provider_id provider id
 *        * @param[in] pool Argobots pool
 *         * @param[out] provider provider handle
 *          *
 *           * @return COMPUTE_SUCCESS or error code defined in compute-common.h
 *            */
int compute_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        compute_provider_t* provider);

/**
 *  * @brief Destroys the Alpha provider and deregisters its RPC.
 *   *
 *    * @param[in] provider Alpha provider
 *     *
 *      * @return COMPUTE_SUCCESS or error code defined in compute-common.h
 *       */
int compute_provider_destroy(
        compute_provider_t provider);

/**
 *  * @brief Creates handles and connections to downstream microservice dependencies
 *   *
 *    * @param[in] mid Margo instance
 *     *  @param[in] p provider id
 *      *  @param[in] svr_addr server address
 *       *
 *        * @return COMPUTE_SUCCESS or error code defined in network-common.h
 *         */
void compute_create_downstream_handles(
	margo_instance_id mid, 
	uint16_t p, 
	hg_addr_t svr_addr);
#ifdef __cplusplus
}
#endif

#endif

