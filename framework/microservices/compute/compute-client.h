#ifndef __COMPUTE_CLIENT_H
#define __COMPUTE_CLIENT_H

#include <margo.h>
#include "../common.h"
#include "compute-common.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct compute_client* compute_client_t;
#define COMPUTE_CLIENT_NULL ((compute_client_t)NULL)

typedef struct compute_provider_handle *compute_provider_handle_t;
#define COMPUTE_PROVIDER_HANDLE_NULL ((compute_provider_handle_t)NULL)

/**
 *  * @brief Creates a COMPUTE client.
 *   *
 *    * @param[in] mid Margo instance
 *     * @param[out] client COMPUTE client
 *      *
 *       * @return COMPUTE_SUCCESS or error code defined in compute-common.h
 *        */
int compute_client_init(margo_instance_id mid, compute_client_t* client);

/**
 *  * @brief Finalizes a COMPUTE client.
 *   *
 *    * @param[in] client COMPUTE client to finalize
 *     *
 *      * @return COMPUTE_SUCCESS or error code defined in compute-common.h
 *       */
int compute_client_finalize(compute_client_t client);

/**
 *  * @brief Creates a COMPUTE provider handle.
 *   *
 *    * @param[in] client COMPUTE client responsible for the provider handle
 *     * @param[in] addr Mercury address of the provider
 *      * @param[in] provider_id id of the provider
 *       * @param[in] handle provider handle
 *        *
 *         * @return COMPUTE_SUCCESS or error code defined in compute-common.h
 *          */
int compute_provider_handle_create(
        compute_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        compute_provider_handle_t* handle);

/**
 *  * @brief Increments the reference counter of a provider handle.
 *   *
 *    * @param handle provider handle
 *     *
 *      * @return COMPUTE_SUCCESS or error code defined in compute-common.h
 *       */
int compute_provider_handle_ref_incr(
        compute_provider_handle_t handle);

/**
 *  * @brief Releases the provider handle. This will decrement the
 *   * reference counter, and free the provider handle if the reference
 *    * counter reaches 0.
 *     *
 *      * @param[in] handle provider handle to release.
 *       *
 *        * @return COMPUTE_SUCCESS or error code defined in compute-common.h
 *         */
int compute_provider_handle_release(compute_provider_handle_t handle);

/**
 *  * @brief Makes the target COMPUTE provider compute the sum of the
 *   * two numbers and return the result.
 *    *
 *     * @param[in] handle provide handle.
 *      * @param[in] workload_factor workload factor.
 *       * @param[in] bulk bulk address.
 *        * @param[in] request_structure request structure
 *           * @param[out] result resulting value.
 *            *
 *             * @return COMPUTE_SUCCESS or error code defined in compute-common.h
 *              */
int compute_do_work(
        compute_provider_handle_t handle,
        int32_t workload_factor,
        hg_bulk_t bulk,
	hg_string_t request_structure,
        int32_t* result);

#endif
