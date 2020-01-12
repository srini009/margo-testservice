#ifndef __DELTA_CLIENT_H
#define __DELTA_CLIENT_H

#include <margo.h>
#include "delta-common.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct delta_client* delta_client_t;
#define DELTA_CLIENT_NULL ((delta_client_t)NULL)

typedef struct delta_provider_handle *delta_provider_handle_t;
#define DELTA_PROVIDER_HANDLE_NULL ((delta_provider_handle_t)NULL)

/**
 *  * @brief Creates a DELTA client.
 *   *
 *    * @param[in] mid Margo instance
 *     * @param[out] client DELTA client
 *      *
 *       * @return DELTA_SUCCESS or error code defined in delta-common.h
 *        */
int delta_client_init(margo_instance_id mid, delta_client_t* client);

/**
 *  * @brief Finalizes a DELTA client.
 *   *
 *    * @param[in] client DELTA client to finalize
 *     *
 *      * @return DELTA_SUCCESS or error code defined in delta-common.h
 *       */
int delta_client_finalize(delta_client_t client);

/**
 *  * @brief Creates a DELTA provider handle.
 *   *
 *    * @param[in] client DELTA client responsible for the provider handle
 *     * @param[in] addr Mercury address of the provider
 *      * @param[in] provider_id id of the provider
 *       * @param[in] handle provider handle
 *        *
 *         * @return DELTA_SUCCESS or error code defined in delta-common.h
 *          */
int delta_provider_handle_create(
        delta_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        delta_provider_handle_t* handle);

/**
 *  * @brief Increments the reference counter of a provider handle.
 *   *
 *    * @param handle provider handle
 *     *
 *      * @return DELTA_SUCCESS or error code defined in delta-common.h
 *       */
int delta_provider_handle_ref_incr(
        delta_provider_handle_t handle);

/**
 *  * @brief Releases the provider handle. This will decrement the
 *   * reference counter, and free the provider handle if the reference
 *    * counter reaches 0.
 *     *
 *      * @param[in] handle provider handle to release.
 *       *
 *        * @return DELTA_SUCCESS or error code defined in delta-common.h
 *         */
int delta_provider_handle_release(delta_provider_handle_t handle);

/**
 *  * @brief Makes the target DELTA provider compute the sum of the
 *   * two numbers and return the result.
 *    *
 *     * @param[in] handle provide handle.
 *      * @param[in] x first number.
 *       * @param[in] y second number.
 *        * @param[out] result resulting value.
 *         *
 *          * @return DELTA_SUCCESS or error code defined in delta-common.h
 *           */
int delta_do_work(
        delta_provider_handle_t handle,
        int32_t n,
        hg_bulk_t bulk,
        int32_t* result);

#endif
