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

#ifndef ONEAPI_DNNL_DNNL_GRAPH_SYCL_HPP
#define ONEAPI_DNNL_DNNL_GRAPH_SYCL_HPP

/// @cond DO_NOT_DOCUMENT_THIS
#include <vector>

#if __has_include(<sycl/sycl.hpp>)
#include <sycl/sycl.hpp>
#elif __has_include(<CL/sycl.hpp>)
#include <CL/sycl.hpp>
#else
#error "Unsupported compiler"
#endif

#include "oneapi/dnnl/dnnl_graph.hpp"
#include "oneapi/dnnl/dnnl_graph_sycl.h"
/// @endcond

/// @addtogroup dnnl_graph_api
/// @{

namespace dnnl {
namespace graph {

/// @addtogroup dnnl_graph_api_interop Runtime interoperability API
/// API extensions to interact with the underlying run-time.
/// @{

/// @addtogroup dnnl_graph_api_sycl_interop SYCL interoperability API
/// API extensions to interact with the underlying SYCL run-time.
///
/// @{

/// SYCL interoperability namespace
namespace sycl_interop {

/// Constructs an allocator from SYCL malloc and free function pointer
///
/// @param sycl_malloc The pointer to SYCL malloc function
/// @param sycl_free The pointer to SYCL free function
/// @returns Created allocator
inline allocator make_allocator(dnnl_graph_sycl_allocate_f sycl_malloc,
        dnnl_graph_sycl_deallocate_f sycl_free) {
    dnnl_graph_allocator_t c_allocator = nullptr;
    error::check_succeed(dnnl_graph_sycl_interop_allocator_create(
                                 &c_allocator, sycl_malloc, sycl_free),
            "could not create allocator for sycl device");
    return allocator(c_allocator);
}

/// Constructs an engine from SYCL device and context objects.
///
/// @param adevice SYCL device.
/// @param acontext SYCL context.
///
/// @returns Created engine.
inline engine make_engine(
        const sycl::device &adevice, const sycl::context &acontext) {
    dnnl_graph_engine_t c_engine = nullptr;
    error::check_succeed(dnnl_graph_sycl_interop_engine_create(&c_engine,
                                 static_cast<const void *>(&adevice),
                                 static_cast<const void *>(&acontext)),
            "could not create engine from sycl device and context");
    return engine(c_engine);
}

/// Constructs an engine from SYCL device, context, and allocator.
///
/// @param adevice SYCL device.
/// @param acontext SYCL context.
/// @param alloc The memory allocator associated to the engine.
///
/// @returns Created engine.
inline engine make_engine(const sycl::device &adevice,
        const sycl::context &acontext, const allocator &alloc) {
    dnnl_graph_engine_t c_engine = nullptr;
    error::check_succeed(
            dnnl_graph_sycl_interop_engine_create_with_allocator(&c_engine,
                    static_cast<const void *>(&adevice),
                    static_cast<const void *>(&acontext), alloc.get()),
            "could not create engine from sycl device and context");
    return engine(c_engine);
}

/// Creates an execution stream for a given engine associated with a SYCL
/// queue.
///
/// @param aengine Engine object to use for the stream.
/// @param aqueue SYCL queue to use for the stream.
///
/// @returns An execution stream.
inline stream make_stream(engine aengine, const sycl::queue &aqueue) {
    dnnl_graph_stream_t c_stream = nullptr;
    error::check_succeed(
            dnnl_graph_sycl_interop_stream_create(&c_stream, aengine.get(),
                    static_cast<const void *>(&aqueue)),
            "could not create stream from sycl queue");
    return stream(c_stream);
}

/// Executes a compiled partition in a specified stream and returns a SYCL
/// event.
///
/// @param c_partition Compiled partition to execute.
/// @param astream Stream object to run over
/// @param inputs Arguments map.
/// @param outputs Arguments map.
/// @param deps Optional vector with `sycl::event` dependencies.
/// @returns Output event.
inline sycl::event execute(compiled_partition &c_partition, stream &astream,
        const std::vector<tensor> &inputs, std::vector<tensor> &outputs,
        const std::vector<sycl::event> &deps = {}) {
    std::vector<const_dnnl_graph_tensor_t> c_inputs;
    c_inputs.reserve(inputs.size());
    for (auto &in : inputs) {
        c_inputs.push_back(in.get());
    }
    std::vector<const_dnnl_graph_tensor_t> c_outputs;
    c_outputs.reserve(outputs.size());
    for (auto &out : outputs) {
        c_outputs.push_back(out.get());
    }

    sycl::event sycl_event;
    error::check_succeed(
            dnnl_graph_sycl_interop_compiled_partition_execute(
                    c_partition.get(), astream.get(), c_inputs.size(),
                    c_inputs.data(), c_outputs.size(), c_outputs.data(), &deps,
                    &sycl_event),
            "could not execute the compiled_partition on a specified sycl "
            "stream");
    return sycl_event;
}

} // namespace sycl_interop

/// @} dnnl_graph_api_sycl_interop

/// @} dnnl_graph_api_interop

} // namespace graph
} // namespace dnnl

/// @} dnnl_graph_api

#endif
