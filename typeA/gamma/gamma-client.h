#ifndef __GAMMA_CLIENT_H
#define __GAMMA_CLIENT_H

#include <margo.h>
#include "gamma-common.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct gamma_client* gamma_client_t;
#define GAMMA_CLIENT_NULL ((gamma_client_t)NULL)

typedef struct gamma_provider_handle *gamma_provider_handle_t;
#define GAMMA_PROVIDER_HANDLE_NULL ((gamma_provider_handle_t)NULL)

/**
 *  * @brief Creates a GAMMA client.
 *   *
 *    * @param[in] mid Margo instance
 *     * @param[out] client GAMMA client
 *      *
 *       * @return GAMMA_SUCCESS or error code defined in gamma-common.h
 *        */
int gamma_client_init(margo_instance_id mid, gamma_client_t* client);

/**
 *  * @brief Finalizes a GAMMA client.
 *   *
 *    * @param[in] client GAMMA client to finalize
 *     *
 *      * @return GAMMA_SUCCESS or error code defined in gamma-common.h
 *       */
int gamma_client_finalize(gamma_client_t client);

/**
 *  * @brief Creates a GAMMA provider handle.
 *   *
 *    * @param[in] client GAMMA client responsible for the provider handle
 *     * @param[in] addr Mercury address of the provider
 *      * @param[in] provider_id id of the provider
 *       * @param[in] handle provider handle
 *        *
 *         * @return GAMMA_SUCCESS or error code defined in gamma-common.h
 *          */
int gamma_provider_handle_create(
        gamma_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        gamma_provider_handle_t* handle);

/**
 *  * @brief Increments the reference counter of a provider handle.
 *   *
 *    * @param handle provider handle
 *     *
 *      * @return GAMMA_SUCCESS or error code defined in gamma-common.h
 *       */
int gamma_provider_handle_ref_incr(
        gamma_provider_handle_t handle);

/**
 *  * @brief Releases the provider handle. This will decrement the
 *   * reference counter, and free the provider handle if the reference
 *    * counter reaches 0.
 *     *
 *      * @param[in] handle provider handle to release.
 *       *
 *        * @return GAMMA_SUCCESS or error code defined in gamma-common.h
 *         */
int gamma_provider_handle_release(gamma_provider_handle_t handle);

/**
 *  * @brief Makes the target GAMMA provider compute the sum of the
 *   * two numbers and return the result.
 *    *
 *     * @param[in] handle provide handle.
 *      * @param[in] x first number.
 *       * @param[in] y second number.
 *        * @param[in] c c
 *         * @param[in] m m
 *          * @param[in] f f
 *           * @param[out] result resulting value.
 *            *
 *             * @return GAMMA_SUCCESS or error code defined in gamma-common.h
 *              */
int gamma_do_work(
        gamma_provider_handle_t handle,
        int32_t n,
        hg_bulk_t bulk,
        int32_t compute,
        int32_t memory,
        int32_t file_size,
        int32_t* result);

#endif
