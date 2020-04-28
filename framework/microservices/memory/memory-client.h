#ifndef __MEMORY_CLIENT_H
#define __MEMORY_CLIENT_H

#include <margo.h>
#include "../common.h"
#include "memory-common.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct memory_client* memory_client_t;
#define MEMORY_CLIENT_NULL ((memory_client_t)NULL)

typedef struct memory_provider_handle *memory_provider_handle_t;
#define MEMORY_PROVIDER_HANDLE_NULL ((memory_provider_handle_t)NULL)

/**
 *  * @brief Creates a MEMORY client.
 *   *
 *    * @param[in] mid Margo instance
 *     * @param[out] client MEMORY client
 *      *
 *       * @return MEMORY_SUCCESS or error code defined in memory-common.h
 *        */
int memory_client_init(margo_instance_id mid, memory_client_t* client);

/**
 *  * @brief Finalizes a MEMORY client.
 *   *
 *    * @param[in] client MEMORY client to finalize
 *     *
 *      * @return MEMORY_SUCCESS or error code defined in memory-common.h
 *       */
int memory_client_finalize(memory_client_t client);

/**
 *  * @brief Creates a MEMORY provider handle.
 *   *
 *    * @param[in] client MEMORY client responsible for the provider handle
 *     * @param[in] addr Mercury address of the provider
 *      * @param[in] provider_id id of the provider
 *       * @param[in] handle provider handle
 *        *
 *         * @return MEMORY_SUCCESS or error code defined in memory-common.h
 *          */
int memory_provider_handle_create(
        memory_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        memory_provider_handle_t* handle);

/**
 *  * @brief Increments the reference counter of a provider handle.
 *   *
 *    * @param handle provider handle
 *     *
 *      * @return MEMORY_SUCCESS or error code defined in memory-common.h
 *       */
int memory_provider_handle_ref_incr(
        memory_provider_handle_t handle);

/**
 *  * @brief Releases the provider handle. This will decrement the
 *   * reference counter, and free the provider handle if the reference
 *    * counter reaches 0.
 *     *
 *      * @param[in] handle provider handle to release.
 *       *
 *        * @return MEMORY_SUCCESS or error code defined in memory-common.h
 *         */
int memory_provider_handle_release(memory_provider_handle_t handle);

/**
 *  * @brief Makes the target MEMORY provider compute the sum of the
 *   * two numbers and return the result.
 *    *
 *     * @param[in] handle provide handle.
 *      * @param[in] workload_factor workload factor.
 *       * @param[in] bulk bulk address.
 *        * @param[in] request_structure request structure
 *           * @param[out] result resulting value.
 *            *
 *             * @return MEMORY_SUCCESS or error code defined in memory-common.h
 *              */
int memory_do_work(
        memory_provider_handle_t handle,
        int32_t workload_factor,
        hg_bulk_t bulk,
	hg_string_t request_structure,
        int32_t* result);

#endif
