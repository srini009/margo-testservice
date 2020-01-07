#ifndef __BETA_CLIENT_H
#define __BETA_CLIENT_H

#include <margo.h>
#include "beta-common.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct beta_client* beta_client_t;
#define BETA_CLIENT_NULL ((beta_client_t)NULL)

typedef struct beta_provider_handle *beta_provider_handle_t;
#define BETA_PROVIDER_HANDLE_NULL ((beta_provider_handle_t)NULL)

/**
 *  * @brief Creates a BETA client.
 *   *
 *    * @param[in] mid Margo instance
 *     * @param[out] client BETA client
 *      *
 *       * @return BETA_SUCCESS or error code defined in beta-common.h
 *        */
int beta_client_init(margo_instance_id mid, beta_client_t* client);

/**
 *  * @brief Finalizes a BETA client.
 *   *
 *    * @param[in] client BETA client to finalize
 *     *
 *      * @return BETA_SUCCESS or error code defined in beta-common.h
 *       */
int beta_client_finalize(beta_client_t client);

/**
 *  * @brief Creates a BETA provider handle.
 *   *
 *    * @param[in] client BETA client responsible for the provider handle
 *     * @param[in] addr Mercury address of the provider
 *      * @param[in] provider_id id of the provider
 *       * @param[in] handle provider handle
 *        *
 *         * @return BETA_SUCCESS or error code defined in beta-common.h
 *          */
int beta_provider_handle_create(
        beta_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        beta_provider_handle_t* handle);

/**
 *  * @brief Increments the reference counter of a provider handle.
 *   *
 *    * @param handle provider handle
 *     *
 *      * @return BETA_SUCCESS or error code defined in beta-common.h
 *       */
int beta_provider_handle_ref_incr(
        beta_provider_handle_t handle);

/**
 *  * @brief Releases the provider handle. This will decrement the
 *   * reference counter, and free the provider handle if the reference
 *    * counter reaches 0.
 *     *
 *      * @param[in] handle provider handle to release.
 *       *
 *        * @return BETA_SUCCESS or error code defined in beta-common.h
 *         */
int beta_provider_handle_release(beta_provider_handle_t handle);

/**
 *  * @brief Makes the target BETA provider compute the sum of the
 *   * two numbers and return the result.
 *    *
 *     * @param[in] handle provide handle.
 *      * @param[in] x first number.
 *       * @param[in] y second number.
 *        * @param[out] result resulting value.
 *         *
 *          * @return BETA_SUCCESS or error code defined in beta-common.h
 *           */
int beta_compute_sum(
        beta_provider_handle_t handle,
        int32_t x,
        int32_t y,
        int32_t* result);

#endif
