/*******************************************************************************
* Copyright 2020-2022 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#ifndef TEST_THREAD_HPP
#define TEST_THREAD_HPP

#include "oneapi/dnnl/dnnl_config.h"

#ifdef COMMON_DNNL_THREAD_HPP
#error "src/common/dnnl_thread.hpp" was already included
#endif

#if DNNL_CPU_RUNTIME == DNNL_RUNTIME_NONE

#if DNNL_CPU_THREADING_RUNTIME != DNNL_RUNTIME_SEQ
#error "DNNL_CPU_THREADING_RUNTIME is expected to be SEQ for GPU only configurations."
#endif

#undef DNNL_CPU_THREADING_RUNTIME

// Enable CPU threading layer for testing:
// - DPCPP: TBB
// - OCL: OpenMP
#if DNNL_GPU_RUNTIME == DNNL_RUNTIME_SYCL
#define DNNL_CPU_THREADING_RUNTIME DNNL_RUNTIME_TBB
#elif DNNL_GPU_RUNTIME == DNNL_RUNTIME_OCL
#define DNNL_CPU_THREADING_RUNTIME DNNL_RUNTIME_OMP
#endif

#endif

// This hack renames the namespaces used by threading functions for
// threadpool-related functions so that the calls to dnnl::impl::parallel*()
// from the test use a special testing threadpool.
//
// At the same time, the calls to dnnl::impl::parallel*() from within the
// library continue using the library version of these functions.
#define threadpool_utils testing_threadpool_utils
#include "src/common/dnnl_thread.hpp"
#undef threadpool_utils

#if DNNL_CPU_RUNTIME == DNNL_RUNTIME_NONE
// Restore the original DNNL_CPU_THREADING_RUNTIME value.
#undef DNNL_CPU_THREADING_RUNTIME
#define DNNL_CPU_THREADING_RUNTIME DNNL_RUNTIME_SEQ
#endif

#ifndef COMMON_DNNL_THREAD_HPP
#error "src/common/dnnl_thread.hpp" has an unexpected header guard
#endif

#if DNNL_CPU_THREADING_RUNTIME == DNNL_RUNTIME_THREADPOOL
#include "oneapi/dnnl/dnnl_threadpool_iface.hpp"
namespace dnnl {

// Original threadpool utils are used by the scoped_tp_activation_t and thus
// need to be re-declared because of the hack above.
namespace impl {
namespace threadpool_utils {
void activate_threadpool(dnnl::threadpool_interop::threadpool_iface *tp);
void deactivate_threadpool();
dnnl::threadpool_interop::threadpool_iface *get_active_threadpool();
int get_max_concurrency();
} // namespace threadpool_utils
} // namespace impl

namespace testing {

dnnl::threadpool_interop::threadpool_iface *get_threadpool();

// Sets the testing threadpool as active for the lifetime of the object.
// Required for the tests that throw to work.
struct scoped_tp_activation_t {
    scoped_tp_activation_t() {
        impl::threadpool_utils::activate_threadpool(get_threadpool());
    }
    ~scoped_tp_activation_t() {
        impl::threadpool_utils::deactivate_threadpool();
    }
};

struct scoped_tp_deactivation_t {
    scoped_tp_deactivation_t() {
        impl::threadpool_utils::deactivate_threadpool();
    }
    ~scoped_tp_deactivation_t() {
        // we always use the same threadpool that is returned by `get_threadpool()`
        impl::threadpool_utils::activate_threadpool(get_threadpool());
    }
};

} // namespace testing
} // namespace dnnl
#endif

#endif
