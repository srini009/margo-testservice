#ifndef __STORAGE_CLIENT_H
#define __STORAGE_CLIENT_H

#include <margo.h>
#include "../common.h"
#include "storage-common.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct storage_client* storage_client_t;
#define STORAGE_CLIENT_NULL ((storage_client_t)NULL)

typedef struct storage_provider_handle *storage_provider_handle_t;
#define STORAGE_PROVIDER_HANDLE_NULL ((storage_provider_handle_t)NULL)

/**
 *  * @brief Creates a STORAGE client.
 *   *
 *    * @param[in] mid Margo instance
 *     * @param[out] client STORAGE client
 *      *
 *       * @return STORAGE_SUCCESS or error code defined in storage-common.h
 *        */
int storage_client_init(margo_instance_id mid, storage_client_t* client);

/**
 *  * @brief Finalizes a STORAGE client.
 *   *
 *    * @param[in] client STORAGE client to finalize
 *     *
 *      * @return STORAGE_SUCCESS or error code defined in storage-common.h
 *       */
int storage_client_finalize(storage_client_t client);

/**
 *  * @brief Creates a STORAGE provider handle.
 *   *
 *    * @param[in] client STORAGE client responsible for the provider handle
 *     * @param[in] addr Mercury address of the provider
 *      * @param[in] provider_id id of the provider
 *       * @param[in] handle provider handle
 *        *
 *         * @return STORAGE_SUCCESS or error code defined in storage-common.h
 *          */
int storage_provider_handle_create(
        storage_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        storage_provider_handle_t* handle);

/**
 *  * @brief Increments the reference counter of a provider handle.
 *   *
 *    * @param handle provider handle
 *     *
 *      * @return STORAGE_SUCCESS or error code defined in storage-common.h
 *       */
int storage_provider_handle_ref_incr(
        storage_provider_handle_t handle);

/**
 *  * @brief Releases the provider handle. This will decrement the
 *   * reference counter, and free the provider handle if the reference
 *    * counter reaches 0.
 *     *
 *      * @param[in] handle provider handle to release.
 *       *
 *        * @return STORAGE_SUCCESS or error code defined in storage-common.h
 *         */
int storage_provider_handle_release(storage_provider_handle_t handle);

/**
 *  * @brief Makes the target STORAGE provider compute the sum of the
 *   * two numbers and return the result.
 *    *
 *     * @param[in] handle provide handle.
 *      * @param[in] workload_factor workload factor.
 *       * @param[in] bulk bulk address.
 *        * @param[in] request_structure request structure
 *           * @param[out] result resulting value.
 *            *
 *             * @return STORAGE_SUCCESS or error code defined in storage-common.h
 *              */
int storage_do_work(
        storage_provider_handle_t handle,
        int32_t workload_factor,
        hg_bulk_t bulk,
        hg_string_t request_structure,
        int32_t* result);

#endif
