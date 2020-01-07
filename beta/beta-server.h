#ifndef __BETA_SERVER_H
#define __BETA_SERVER_H

#include <margo.h>
#include "beta-common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BETA_ABT_POOL_DEFAULT ABT_POOL_NULL

typedef struct beta_provider* beta_provider_t;
#define BETA_PROVIDER_NULL ((beta_provider_t)NULL)
#define BETA_PROVIDER_IGNORE ((beta_provider_t*)NULL)

/**
 *  * @brief Creates a new BETA provider. If BETA_PROVIDER_IGNORE
 *   * is passed as last argument, the provider will be automatically
 *    * destroyed when calling :code:`margo_finalize`.
 *     *
 *      * @param[in] mid Margo instance
 *       * @param[in] provider_id provider id
 *        * @param[in] pool Argobots pool
 *         * @param[out] provider provider handle
 *          *
 *           * @return BETA_SUCCESS or error code defined in beta-common.h
 *            */
int beta_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        beta_provider_t* provider);

/**
 *  * @brief Destroys the Alpha provider and deregisters its RPC.
 *   *
 *    * @param[in] provider Alpha provider
 *     *
 *      * @return BETA_SUCCESS or error code defined in beta-common.h
 *       */
int beta_provider_destroy(
        beta_provider_t provider);

#ifdef __cplusplus
}
#endif

#endif

