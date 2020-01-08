#ifndef __GAMMA_SERVER_H
#define __GAMMA_SERVER_H

#include <margo.h>
#include "gamma-common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GAMMA_ABT_POOL_DEFAULT ABT_POOL_NULL

typedef struct gamma_provider* gamma_provider_t;
#define GAMMA_PROVIDER_NULL ((gamma_provider_t)NULL)
#define GAMMA_PROVIDER_IGNORE ((gamma_provider_t*)NULL)

/**
 *  * @brief Creates a new GAMMA provider. If GAMMA_PROVIDER_IGNORE
 *   * is passed as last argument, the provider will be automatically
 *    * destroyed when calling :code:`margo_finalize`.
 *     *
 *      * @param[in] mid Margo instance
 *       * @param[in] provider_id provider id
 *        * @param[in] pool Argobots pool
 *         * @param[out] provider provider handle
 *          *
 *           * @return GAMMA_SUCCESS or error code defined in gamma-common.h
 *            */
int gamma_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        gamma_provider_t* provider);

/**
 *  * @brief Destroys the Alpha provider and deregisters its RPC.
 *   *
 *    * @param[in] provider Alpha provider
 *     *
 *      * @return GAMMA_SUCCESS or error code defined in gamma-common.h
 *       */
int gamma_provider_destroy(
        gamma_provider_t provider);

/**
 *  * @brief Creates handles and connections to downstream microservice dependencies
 *   *
 *    * @param[in] mid Margo instance
 *     *  @param[in] p provider id
 *      *  @param[in] svr_addr server address
 *       *
 *        * @return GAMMA_SUCCESS or error code defined in alpha-common.h
 *         */
void gamma_create_downstream_handles(
	margo_instance_id mid, 
	uint16_t p, 
	hg_addr_t svr_addr);
#ifdef __cplusplus
}
#endif

#endif

