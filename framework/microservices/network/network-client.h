#ifndef __NETWORK_CLIENT_H
#define __NETWORK_CLIENT_H

#include <margo.h>
#include "../common.h"
#include "network-common.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct network_client* network_client_t;
#define NETWORK_CLIENT_NULL ((network_client_t)NULL)

typedef struct network_provider_handle *network_provider_handle_t;
#define NETWORK_PROVIDER_HANDLE_NULL ((network_provider_handle_t)NULL)

/**
 *  * @brief Creates a NETWORK client.
 *   *
 *    * @param[in] mid Margo instance
 *     * @param[out] client NETWORK client
 *      *
 *       * @return NETWORK_SUCCESS or error code defined in network-common.h
 *        */
int network_client_init(margo_instance_id mid, network_client_t* client);

/**
 *  * @brief Finalizes a NETWORK client.
 *   *
 *    * @param[in] client NETWORK client to finalize
 *     *
 *      * @return NETWORK_SUCCESS or error code defined in network-common.h
 *       */
int network_client_finalize(network_client_t client);

/**
 *  * @brief Creates a NETWORK provider handle.
 *   *
 *    * @param[in] client NETWORK client responsible for the provider handle
 *     * @param[in] addr Mercury address of the provider
 *      * @param[in] provider_id id of the provider
 *       * @param[in] handle provider handle
 *        *
 *         * @return NETWORK_SUCCESS or error code defined in network-common.h
 *          */
int network_provider_handle_create(
        network_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        network_provider_handle_t* handle);

/**
 *  * @brief Increments the reference counter of a provider handle.
 *   *
 *    * @param handle provider handle
 *     *
 *      * @return NETWORK_SUCCESS or error code defined in network-common.h
 *       */
int network_provider_handle_ref_incr(
        network_provider_handle_t handle);

/**
 *  * @brief Releases the provider handle. This will decrement the
 *   * reference counter, and free the provider handle if the reference
 *    * counter reaches 0.
 *     *
 *      * @param[in] handle provider handle to release.
 *       *
 *        * @return NETWORK_SUCCESS or error code defined in network-common.h
 *         */
int network_provider_handle_release(network_provider_handle_t handle);

/**
 *  * @brief Makes the target NETWORK provider compute the sum of the
 *   * two numbers and return the result.
 *    *
 *     * @param[in] handle provide handle.
 *      * @param[in] x first number.
         * @param[in] compute compute_cycles
          * @param[in] memory memory copy
           * @param[in] file_size file size
 *          * @param[out] result resulting value.
 *           *
 *            * @return NETWORK_SUCCESS or error code defined in network-common.h
 *             */
int network_do_work(
        network_provider_handle_t handle,
        int32_t x,
        hg_bulk_t local_bulk,
        int32_t compute,
        int32_t memory,
        int32_t file_size,
        int32_t* result);

#endif
