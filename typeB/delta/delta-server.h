#ifndef __DELTA_SERVER_H
#define __DELTA_SERVER_H

#include <margo.h>
#include "delta-common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DELTA_ABT_POOL_DEFAULT ABT_POOL_NULL

typedef struct delta_provider* delta_provider_t;
#define DELTA_PROVIDER_NULL ((delta_provider_t)NULL)
#define DELTA_PROVIDER_IGNORE ((delta_provider_t*)NULL)

/**
 *  * @brief Creates a new DELTA provider. If DELTA_PROVIDER_IGNORE
 *   * is passed as last argument, the provider will be automatically
 *    * destroyed when calling :code:`margo_finalize`.
 *     *
 *      * @param[in] mid Margo instance
 *       * @param[in] provider_id provider id
 *        * @param[in] pool Argobots pool
 *         * @param[out] provider provider handle
 *          *
 *           * @return DELTA_SUCCESS or error code defined in delta-common.h
 *            */
int delta_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        delta_provider_t* provider);

/**
 *  * @brief Destroys the Alpha provider and deregisters its RPC.
 *   *
 *    * @param[in] provider Alpha provider
 *     *
 *      * @return DELTA_SUCCESS or error code defined in delta-common.h
 *       */
int delta_provider_destroy(
        delta_provider_t provider);

/**
 *  * @brief Creates handles and connections to downstream microservice dependencies
 *   *
 *    * @param[in] mid Margo instance
 *     *  @param[in] p provider id
 *      *  @param[in] svr_addr server address
 *       *
 *        * @return DELTA_SUCCESS or error code defined in delta-common.h
 *         */
void delta_create_downstream_handles(
	margo_instance_id mid, 
	uint16_t p, 
	hg_addr_t svr_addr);
#ifdef __cplusplus
}
#endif

#endif

