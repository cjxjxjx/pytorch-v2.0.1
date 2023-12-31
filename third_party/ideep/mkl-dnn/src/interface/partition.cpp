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

#include <cassert>
#include <cstring>
#include <limits>
#include <set>
#include <sstream>
#include <thread>

#include "oneapi/dnnl/dnnl_graph.h"
#include "oneapi/dnnl/dnnl_graph_sycl.h"

#include "interface/backend.hpp"
#include "interface/graph.hpp"
#include "interface/logical_tensor.hpp"
#include "interface/op_schema.hpp"
#include "interface/partition.hpp"
#include "interface/partition_cache.hpp"

#ifdef DNNL_GRAPH_WITH_SYCL
#include "utils/sycl_utils.hpp"
#endif

using namespace dnnl::graph::impl;

status_t DNNL_GRAPH_API dnnl_graph_partition_create(partition_t **partition) {
    if (partition == nullptr) return status::invalid_arguments;

    *partition = new partition_t();
    return status::success;
}

/// This allows to create a partition directly with an op and an engine kind.
/// In order to not break backend API and change the existing graph and
/// partition implementation, we internally construct a temporal graph object,
/// add the operator to it, and then do partitioning on the graph. The workflow
/// should be the same as partitioning a normal user graph.
status_t DNNL_GRAPH_API dnnl_graph_partition_create_with_op(
        partition_t **partition, const op_t *op, engine_kind_t ekind) {
    using ltw = logical_tensor_wrapper_t;

    if (utils::any_null(partition, op)) return status::invalid_arguments;

    // new an empty partition
    *partition = new partition_t();

    status_t ret = status::success;

    // construct a single op graph
    graph_t g {ekind};
    ret = g.add_op(op);

    if (ret != status::success) return ret;

    // find opaque logical tensor in inputs
    const auto &input_vals = op->get_input_values();
    auto opaque_in_iter = std::find_if(input_vals.begin(), input_vals.end(),
            [](const std::shared_ptr<value_t> &it) {
                return ltw(it->get_logical_tensor()).is_opaque();
            });
    // find opaque layout tensors in outputs
    const auto &output_vals = op->get_output_values();
    auto opaque_out_iter = std::find_if(output_vals.begin(), output_vals.end(),
            [](const std::shared_ptr<value_t> &it) {
                return ltw(it->get_logical_tensor()).is_opaque();
            });

    // Case 1: all input/outputs are not opaque logical tensors
    // we need go through all registered backends to get partitions
    if (opaque_in_iter == input_vals.end()
            && opaque_out_iter == output_vals.end()) {
        // get partition impl. by calling each backend
        std::vector<const backend *> &backends
                = backend_registry_t::get_singleton().get_registered_backends();
        for (const auto &cbkd : backends) {
            backend *bkd = const_cast<backend *>(cbkd);
            ret = bkd->get_partitions(g, partition_policy::fusion);
            if (ret != status::success) return ret;
        }
    } else {
        // Case 2: if input/output logical tensors have already embedded with
        // backend ID (e.g opaque layout), here we directly use the same backend
        // to get partitions
        bool in_has_valid_layout_id = opaque_in_iter != input_vals.end();
        bool out_has_valid_layout_id = opaque_out_iter != output_vals.end();
        size_t in_valid_layout_id = in_has_valid_layout_id
                ? ltw((*opaque_in_iter)->get_logical_tensor()).layout_id()
                : std::numeric_limits<size_t>::max();
        size_t out_valid_layout_id = out_has_valid_layout_id
                ? ltw((*opaque_out_iter)->get_logical_tensor()).layout_id()
                : std::numeric_limits<size_t>::max();
        if (in_has_valid_layout_id && out_has_valid_layout_id) {
            size_t in_backend_id = backend_registry_t::extract_backend_id(
                    in_valid_layout_id);
            size_t out_backend_id = backend_registry_t::extract_backend_id(
                    out_valid_layout_id);
            // input and output logical tensor have different backend IDs
            if (in_backend_id != out_backend_id) {
                assertm(false, "backends dismatch between inputs and outputs");
                return status::unimplemented;
            }
        }
        size_t valid_layout_id = in_has_valid_layout_id ? in_valid_layout_id
                                                        : out_valid_layout_id;
        backend *bkd = const_cast<backend *>(
                backend_registry_t::get_singleton().get_registered_backend(
                        valid_layout_id));
        assertm(bkd != nullptr,
                "backend is not valid since layout id maybe not correct.");
        ret = bkd->get_partitions(g, partition_policy::fusion);
        if (ret != status::success) return ret;
    }

    // check the partition impl.
    auto &partition_vec = g.get_partitions();
    assertm(partition_vec.size() == 1,
            "single op graph should contain only one partition");
    if (partition_vec[0]->get_assigned_backend() == nullptr) {
        return status::invalid_graph;
    }

    // wrap into the partition
    std::vector<partition_t *> parts {*partition};
    g.get_ordered_partitions(parts);
    return ret;
}

status_t DNNL_GRAPH_API dnnl_graph_partition_destroy(partition_t *partition) {
    delete partition;
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_partition_get_op_num(
        const partition_t *partition, size_t *num) {
    if (utils::any_null(partition, num)) return status::invalid_arguments;

    *num = partition->num_ops();
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_partition_get_ops(
        partition_t *partition, size_t num, size_t *ops) {
    if (utils::any_null(partition, ops)) { return status::invalid_arguments; }

    auto ids = partition->get_op_ids();
    if (ids.size() != num) { return status::invalid_arguments; }

    int idx = 0;
    for (auto it = ids.begin(); it != ids.end(); ++it, ++idx) {
        ops[idx] = *it;
    }

    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_partition_get_id(
        const partition_t *partition, size_t *id) {
    if (utils::any_null(partition, id)) { return status::invalid_arguments; }

    *id = partition->id();
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_partition_compile(partition_t *partition,
        compiled_partition_t *compiled_partition, size_t in_num,
        const logical_tensor_t **inputs, size_t out_num,
        const logical_tensor_t **outputs, const engine_t *engine) {
    if (utils::any_null(partition, compiled_partition, engine)) {
        return status::invalid_arguments;
    }

    if (!partition->is_supported()) return status::invalid_arguments;

    std::vector<const logical_tensor_t *> in {inputs, inputs + in_num};
    std::vector<const logical_tensor_t *> out {outputs, outputs + out_num};

    // The boolean in the pair indicates whether the compiled partition is from
    // global cache.
    //   true - cache_hit, the compiled partition is in the cache
    //   false - cache_miss, the compiled partition is not in the cache
    std::pair<compiled_partition_t *, bool> cp {compiled_partition, false};

    if (utils::get_verbose() >= 2) {
        double ms = utils::get_msec();
        CHECK(partition->compile(cp, in, out, engine, nullptr));
        ms = utils::get_msec() - ms;

        const char *cache_status = cp.second ? "cache_hit" : "cache_miss";
        printf("onednn_graph_verbose,compile:%s,%s,%g\n", cache_status,
                compiled_partition->info(), ms);
        fflush(stdout);
    } else {
        CHECK(partition->compile(cp, in, out, engine, nullptr));
    }
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_partition_compile_v2(partition_t *partition,
        compiled_partition_t *compiled_partition, size_t in_num,
        const logical_tensor_t **inputs, size_t out_num,
        const logical_tensor_t **outputs, const engine_t *engine,
        const context_t *context) {
    if (utils::any_null(partition, compiled_partition, engine)) {
        return status::invalid_arguments;
    }

    if (!partition->is_supported()) return status::invalid_arguments;

    std::vector<const logical_tensor_t *> in {inputs, inputs + in_num};
    std::vector<const logical_tensor_t *> out {outputs, outputs + out_num};

    // The boolean in the pair indicates whether the compiled partition is from
    // global cache.
    //   true - cache_hit, the compiled partition is in the cache
    //   false - cache_miss, the compiled partition is not in the cache
    std::pair<compiled_partition_t *, bool> cp {compiled_partition, false};

    if (utils::get_verbose() >= 2) {
        double ms = utils::get_msec();
        CHECK(partition->compile(cp, in, out, engine, context));
        ms = utils::get_msec() - ms;

        const char *cache_status = cp.second ? "cache_hit" : "cache_miss";
        printf("onednn_graph_verbose,compile:%s,%s,%g\n", cache_status,
                compiled_partition->info(), ms);
        fflush(stdout);
    } else {
        CHECK(partition->compile(cp, in, out, engine, context));
    }
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_partition_get_in_ports_num(
        const partition_t *partition, size_t *num) {
    if (utils::any_null(partition, num)) { return status::invalid_arguments; }

    *num = partition->get_inputs_num();
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_partition_get_out_ports_num(
        const partition_t *partition, size_t *num) {
    if (utils::any_null(partition, num)) { return status::invalid_arguments; }

    *num = partition->get_outputs_num();
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_partition_get_in_ports(
        const partition_t *partition, size_t num, logical_tensor_t *inputs) {
    if (utils::any_null(partition, inputs)
            || partition->get_inputs_num() != num) {
        return status::invalid_arguments;
    }

    auto &in = partition->get_inputs();
    for (size_t i = 0; i < num; ++i) {
        inputs[i] = in[i];
    }

    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_partition_get_out_ports(
        const partition_t *partition, size_t num, logical_tensor_t *outputs) {
    if (utils::any_null(partition, outputs)
            || partition->get_outputs_num() != num) {
        return status::invalid_arguments;
    }

    auto &out = partition->get_outputs();
    for (size_t i = 0; i < num; ++i) {
        outputs[i] = out[i];
    }

    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_partition_is_supported(
        const partition_t *partition, uint8_t *is_supported) {
    if (utils::any_null(partition, is_supported))
        return status::invalid_arguments;

    *is_supported = static_cast<uint8_t>(partition->is_supported());
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_partition_get_engine_kind(
        const partition_t *partition, engine_kind_t *kind) {
    if (utils::any_null(partition, kind)) { return status::invalid_arguments; }

    *kind = partition->get_pimpl()->get_engine_kind();
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_partition_get_kind(
        const partition_t *partition, partition_kind_t *kind) {
    if (utils::any_null(partition, kind)) { return status::invalid_arguments; }

    *kind = partition->get_kind();
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_compilation_context_destroy(
        context_t *context) {
    delete context;
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_compilation_context_create(
        context_t **context) {
    if (context == nullptr) return status::invalid_arguments;

    *context = new context_t();
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_compilation_context_set_tensor_data_handle(
        context_t *context, size_t id, void *handle) {
    if (utils::any_null(context)) return status::invalid_arguments;

    context->set_tensor_data_handle(id, handle);
    return status::success;
}

///
/// dnnl_graph_compiled_partition_t
///
status_t DNNL_GRAPH_API dnnl_graph_compiled_partition_create(
        compiled_partition_t **compiled_partition, partition_t *partition) {
    if (utils::any_null(compiled_partition, partition)) {
        return status::invalid_arguments;
    }

    *compiled_partition = new compiled_partition_t {*partition};
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_compiled_partition_execute(
        const compiled_partition_t *compiled_partition, stream_t *stream,
        size_t num_inputs, const tensor_t **inputs, size_t num_outputs,
        const tensor_t **outputs) {
    if (utils::any_null(stream, compiled_partition, inputs, outputs)) {
        return status::invalid_arguments;
    }

    std::vector<tensor_t> ins, outs;
    ins.reserve(num_inputs);
    outs.reserve(num_outputs);

    for (size_t i = 0; i < num_inputs; ++i) {
        ins.emplace_back(**(inputs + i));
    }
    for (size_t i = 0; i < num_outputs; ++i) {
        outs.emplace_back(**(outputs + i));
    }

#ifndef NDEBUG
    if (utils::get_verbose() >= 3) {
        allocator_t *alloc = compiled_partition->get_engine().get_allocator();
        allocator_t::monitor_t::reset_peak_temp_memory(alloc);
        stream->wait();
        double ms = utils::get_msec();
        CHECK(compiled_partition->execute(stream, ins, outs));
        stream->wait();
        ms = utils::get_msec() - ms;
        printf("onednn_graph_verbose,exec,%s,%g,%zu,%s,%zu,%zu\n",
                compiled_partition->info(), ms, alloc->id(),
                utils::thread_id_to_str(std::this_thread::get_id()).c_str(),
                allocator_t::monitor_t::get_total_persist_memory(alloc),
                allocator_t::monitor_t::get_peak_temp_memory(alloc));
        fflush(stdout);
    } else if (utils::get_verbose()) {
#else
    if (utils::get_verbose()) {
#endif
        stream->wait();
        double ms = utils::get_msec();
        CHECK(compiled_partition->execute(stream, ins, outs));
        stream->wait();
        ms = utils::get_msec() - ms;
        printf("onednn_graph_verbose,exec,%s,%g\n", compiled_partition->info(),
                ms);
        fflush(stdout);
    } else {
        CHECK(compiled_partition->execute(stream, ins, outs));
    }
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_sycl_interop_compiled_partition_execute(
        const compiled_partition_t *compiled_partition, stream_t *stream,
        size_t num_inputs, const tensor_t **inputs, size_t num_outputs,
        const tensor_t **outputs, const void *deps, void *sycl_event) {
#ifdef DNNL_GRAPH_WITH_SYCL
    if (utils::any_null(stream, compiled_partition, inputs, outputs))
        return status::invalid_arguments;
    if (stream->get_engine()->kind() == engine_kind::gpu) {
#ifndef DNNL_GRAPH_GPU_SYCL
        return status::invalid_arguments;
#endif
    } else {
#ifndef DNNL_GRAPH_CPU_SYCL
        return status::invalid_arguments;
#endif
    }

    std::vector<tensor_t> ins, outs;
    ins.reserve(num_inputs);
    outs.reserve(num_outputs);
    for (size_t i = 0; i < num_inputs; ++i) {
        ins.emplace_back(**(inputs + i));
    }
    for (size_t i = 0; i < num_outputs; ++i) {
        outs.emplace_back(**(outputs + i));
    }
#ifndef NDEBUG
    if (utils::get_verbose() >= 3) {
        allocator_t *alloc = compiled_partition->get_engine().get_allocator();
        allocator_t::monitor_t::reset_peak_temp_memory(alloc);
        stream->wait();
        double ms = utils::get_msec();
        if (deps != nullptr) {
            const auto &sycl_deps = *(const std::vector<::sycl::event> *)deps;
            CHECK(compiled_partition->execute_sycl(stream, ins, outs, sycl_deps,
                    static_cast<::sycl::event *>(sycl_event)));
        } else {
            CHECK(compiled_partition->execute_sycl(stream, ins, outs, {},
                    static_cast<::sycl::event *>(sycl_event)));
        }
        stream->wait();
        ms = utils::get_msec() - ms;
        printf("onednn_graph_verbose,exec,%s,%g,%zu,%s,%zu,%zu\n",
                compiled_partition->info(), ms, alloc->id(),
                utils::thread_id_to_str(std::this_thread::get_id()).c_str(),
                allocator_t::monitor_t::get_total_persist_memory(alloc),
                allocator_t::monitor_t::get_peak_temp_memory(alloc));
        fflush(stdout);
    } else if (utils::get_verbose()) {
#else
    if (utils::get_verbose()) {
#endif
        stream->wait();
        double ms = utils::get_msec();
        if (deps != nullptr) {
            const auto &sycl_deps = *(const std::vector<::sycl::event> *)deps;
            CHECK(compiled_partition->execute_sycl(stream, ins, outs, sycl_deps,
                    static_cast<::sycl::event *>(sycl_event)));
        } else {
            CHECK(compiled_partition->execute_sycl(stream, ins, outs, {},
                    static_cast<::sycl::event *>(sycl_event)));
        }
        stream->wait();
        ms = utils::get_msec() - ms;
        printf("onednn_graph_verbose,exec,%s,%g\n", compiled_partition->info(),
                ms);
        fflush(stdout);
    } else {
        if (deps != nullptr) {
            const auto &sycl_deps = *(const std::vector<::sycl::event> *)deps;
            CHECK(compiled_partition->execute_sycl(stream, ins, outs, sycl_deps,
                    static_cast<::sycl::event *>(sycl_event)));
        } else {
            CHECK(compiled_partition->execute_sycl(stream, ins, outs, {},
                    static_cast<::sycl::event *>(sycl_event)));
        }
    }
    return status::success;
#else
    UNUSED(compiled_partition);
    UNUSED(stream);
    UNUSED(num_inputs);
    UNUSED(inputs);
    UNUSED(num_outputs);
    UNUSED(outputs);
    UNUSED(deps);
    UNUSED(sycl_event);
    return status::unimplemented;
#endif
}

status_t DNNL_GRAPH_API dnnl_graph_compiled_partition_destroy(
        compiled_partition_t *compiled_partition) {
    delete compiled_partition;
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_compiled_partition_query_logical_tensor(
        const compiled_partition_t *compiled_partition, size_t tid,
        logical_tensor_t *lt) {
    if (utils::any_null(compiled_partition, lt))
        return status::invalid_arguments;
    return compiled_partition->query_logical_tensor(tid, lt);
}

status_t DNNL_GRAPH_API dnnl_graph_compiled_partition_get_outputs_num(
        const compiled_partition_t *compiled_partition, size_t *num) {
    if (utils::any_null(compiled_partition, num))
        return status::invalid_arguments;

    *num = compiled_partition->get_outputs().size();
    return status::success;
}

status_t DNNL_GRAPH_API dnnl_graph_compiled_partition_query_dynamic_outputs(
        const compiled_partition_t *compiled_partition, size_t num_outputs,
        logical_tensor_t *outputs, size_t num_inputs,
        const logical_tensor_t **inputs, const context_t *context) {
    if (utils::any_null(compiled_partition, outputs))
        return status::invalid_arguments;

    std::vector<const logical_tensor_t *> in_lts {};
    if (num_inputs > 0) {
        in_lts.reserve(num_inputs);
        for (size_t i = 0; i < num_inputs; ++i)
            in_lts.emplace_back(*(inputs + i));
    }

    std::vector<logical_tensor_t *> out_lts {};
    if (num_outputs > 0) {
        out_lts.reserve(num_outputs);
        for (size_t i = 0; i < num_outputs; ++i)
            out_lts.emplace_back(outputs + i);
    }

    return compiled_partition->query_dynamic_outputs(out_lts, in_lts, context);
}

status_t DNNL_GRAPH_API dnnl_graph_compiled_partition_get_inplace_ports(
        const compiled_partition_t *compiled_partition,
        size_t *num_inplace_pairs, const inplace_pair_t **inplace_pairs) {
    if (utils::any_null(compiled_partition, num_inplace_pairs, inplace_pairs))
        return status::invalid_arguments;

    const auto &cp_inplace_pairs = compiled_partition->get_inplace_pairs();
    *num_inplace_pairs = cp_inplace_pairs.size();
    *inplace_pairs = cp_inplace_pairs.data();

    return status::success;
}

impl::status_t dnnl_graph_partition::infer_shape(
        std::vector<const impl::logical_tensor_t *> &inputs,
        std::vector<impl::logical_tensor_t *> &outputs) {
    // check if shape is already known, if so, no need to do shape inference
    auto pos = std::find_if(outputs.begin(), outputs.end(),
            [&](const std::vector<logical_tensor_t *>::value_type &out)
                    -> bool {
                return logical_tensor_wrapper_t(out).is_shape_unknown();
            });
    if (pos == outputs.end()) { return status::success; }

    return pimpl_->infer_shape(inputs, outputs);
}

static status_t pre_process(std::vector<logical_tensor_t> &dst,
        std::vector<const logical_tensor_t *> &src, const backend *abackend) {
    using ltw = logical_tensor_wrapper_t;
    dst.reserve(src.size());
    for (size_t i = 0; i < src.size(); i++) {
        dst.emplace_back(*src[i]);
        if (ltw(src[i]).is_opaque()) {
            size_t layout_id = src[i]->layout.layout_id;
            auto pair = backend_registry_t::decode_layout_id(layout_id);
            if (pair.second != abackend->get_id()) {
                // given opaque layout id must be generated by this
                // backend
                return status::invalid_arguments;
            }
            dst[i].layout.layout_id = pair.first;
        }
    }
    return status::success;
}

static status_t post_process(std::vector<logical_tensor_t> &dst,
        std::vector<logical_tensor_t> &src, const backend *abackend) {
    using ltw = logical_tensor_wrapper_t;
    UNUSED(src);

    for (size_t i = 0; i < dst.size(); i++) {
        if (ltw(dst[i]).is_opaque()) {
            size_t layout_id = dst[i].layout.layout_id;
            dst[i].layout.layout_id = backend_registry_t::encode_layout_id(
                    layout_id, abackend->get_id());
        }
    }
    return status::success;
}

static status_t pre_process(std::vector<tensor_t> &dst,
        const std::vector<tensor_t> &src, const backend *abackend) {
    using ltw = logical_tensor_wrapper_t;
    dst.reserve(src.size());
    for (size_t i = 0; i < src.size(); i++) {
        dst.emplace_back(src[i]);
        auto &src_lt = src[i].get_logical_tensor();
        if (ltw(src_lt).is_opaque()) {
            size_t layout_id = src_lt.layout.layout_id;
            auto pair = backend_registry_t::decode_layout_id(layout_id);
            if (pair.second != abackend->get_id()) {
                return status::invalid_arguments;
            }
            auto &dst_lt = const_cast<logical_tensor_t &>(
                    dst[i].get_logical_tensor());

            dst_lt.layout.layout_id = pair.first;
        }
    }
    return status::success;
}

bool dnnl_graph_partition::is_supported() const {
    return (pimpl_ != nullptr)
            && (pimpl_->get_assigned_backend()->get_name() != "fake_backend");
}

status_t dnnl_graph_partition::compile(compiled_partition_t *cp,
        std::vector<const impl::logical_tensor_t *> &inputs,
        std::vector<const impl::logical_tensor_t *> &outputs,
        const engine_t *aengine, const context_t *acontext) const {
    using ltw = impl::logical_tensor_wrapper_t;
    status_t ret;

    if (!aengine || aengine->kind() != pimpl_->get_engine_kind())
        return status::invalid_arguments;

    const backend *backend = pimpl_->get_assigned_backend();
    if (!backend) return status::invalid_arguments;

    // additional check for dynamic shape cases
    // raise error if input shape are determinable (-1 or any other positive
    // value) at graph building stage while dynamic (-2) at compilation stage.
    for (const auto &part_in : get_inputs()) {
        auto id = part_in.id;
        auto found = std::find_if(inputs.begin(), inputs.end(),
                [&id](const impl::logical_tensor_t *lt) {
                    return id == lt->id;
                });
        if (found == inputs.end() || !ltw(*found).has_dynamic_dim()) continue;

        // ndims is unknown at graph building but dynamic at compilation
        if (ltw(part_in).ndims() < 0) return status::invalid_arguments;
        const auto &part_in_dims = ltw(part_in).vdims();
        const auto &real_dims = ltw(*found).vdims();
        assertm(part_in_dims.size() == real_dims.size(),
                "ndims provided in graph building is different from "
                "compilation.");
        for (size_t i = 0; i < real_dims.size(); ++i) {
            if (ltw::is_dim_dynamic(real_dims[i])
                    && !ltw::is_dim_dynamic(part_in_dims[i])) {
                assertm(false,
                        "dynamic dimension should be consistent at graph "
                        "building and compilation.");
                return status::invalid_arguments;
            }
        }
    }

    // Pre-process the given logical tensor. The pre-process includes
    // 1. decode backend id from the layout id and remove it
    std::vector<logical_tensor_t> tmp_inputs, tmp_outputs;
    ret = pre_process(tmp_inputs, inputs, backend);
    if (status::success != ret) return ret;

    ret = pre_process(tmp_outputs, outputs, backend);
    if (status::success != ret) return ret;

    // Count how many registered backends support the engine kind
    const engine_kind_t kind = aengine->kind();
    size_t effective_backends = 0;
    for (const auto &bkd :
            backend_registry_t::get_singleton().get_registered_backends()) {
        const bool is_not_fake = bkd->get_priority() > 0;
        if (is_not_fake && bkd->support_engine_kind(kind)) {
            effective_backends++;
        }
    }

    // If engine kind is GPU and only dnnl backend supports GPU, we can
    // safely use blocked layout to improve performance. Otherwise, we must
    // use plain layout, since: 1. plain layout usually give optimal layout
    // on CPU. 2. we don't want to pass blocked layout cross backends.
    const bool can_use_blocked_layout
            = effective_backends == 1 && kind == engine_kind::gpu;
    const_cast<partition_impl_t *>(pimpl_.get())
            ->set_use_blocked_layout(can_use_blocked_layout);

#ifdef DNNL_GRAPH_ENABLE_DUMP
    if (utils::getenv_int_user("DUMP", 0) > 1
            || utils::check_verbose_string_user("DUMP", "subgraph")) {
        if (!is_supported()) return status::unimplemented;
        // deep copy for graph serialization
        auto part = pimpl_->clone();
        const std::vector<std::shared_ptr<op_t>> &fused_op = part->get_ops();
        if (fused_op.empty()) return status::invalid_arguments;
        auto agraph = graph_t(fused_op, get_engine_kind(), get_fpmath_mode());
        // set user given logical tensors and infer shape
        agraph.set_user_inputs_outputs(tmp_inputs, tmp_outputs);
        agraph.infer_shape();
        // hash logical tensors to generate unique filename
        impl::partition_hashing::key_t key(this, inputs, outputs);
        size_t seed = 0;
        seed = impl::partition_hashing::get_unordered_array_hash(
                seed, key.ins_);
        seed = impl::partition_hashing::get_unordered_array_hash(
                seed, key.outs_);
        std::stringstream filename;
        filename << "graph-" << id() << "-" << seed << ".json";
        agraph.serialize(filename.str());
    }
#endif

    // The impl's compile will generate the compiled_partition_impl and
    // modify the given inputs outputs logical tensor
    ret = pimpl_->compile(cp, tmp_inputs, tmp_outputs, aengine, acontext);
    if (status::success != ret) return ret;

    // Post-process the modified logical tensor and store them
    // to compiled_partition_impl. The post-process includes
    // 1. encode backend id to generated layout id
    ret = post_process(cp->get_mutable_inputs(), tmp_inputs, backend);
    if (status::success != ret) return ret;

    ret = post_process(cp->get_mutable_outputs(), tmp_outputs, backend);
    if (status::success != ret) return ret;

    if (ret != status::success || !cp->is_initialized())
        return status::unimplemented;
    return status::success;
}

impl::status_t dnnl_graph_partition::compile(
        std::pair<impl::compiled_partition_t *, bool> &compiled_partition,
        std::vector<const impl::logical_tensor_t *> &inputs,
        std::vector<const impl::logical_tensor_t *> &outputs,
        const impl::engine_t *aengine, const impl::context_t *acontext) const {
    namespace partition_hashing = impl::partition_hashing;
    auto &global_compiled_partition_cache = impl::compiled_partition_cache();
    partition_hashing::key_t key(this, inputs, outputs, acontext);

    std::promise<impl::compiled_partition_cache_t::cache_value_t> cp_promise;
    // Try to get the shared future from the cache, if it's missing then
    // a shared future with no shared state is returned and the passed
    // shared future is added, otherwise a valid shared future is returned
    // and no insertion is performed.
    auto cp_future = global_compiled_partition_cache.get_or_add(
            key, cp_promise.get_future());

    bool is_from_cache = cp_future.valid();

    impl::status_t status = impl::status::success;
    std::shared_ptr<impl::compiled_partition_t> cp;

    if (is_from_cache) {
        // The requested compiled partition is present in the cache or is being
        // created by another thread.
        cp = cp_future.get().compiled_partition;
        if (!cp) return cp_future.get().status;
        compiled_partition.first->init(cp->pimpl_);
    } else {
        // The requested compiled partition is NOT present in the cache
        // therefore we have to create it and notify the waiting threads once
        // the creation is done.
        status = this->compile(
                compiled_partition.first, inputs, outputs, aengine, acontext);
        if (status != impl::status::success) {
            // Communicate an error
            cp_promise.set_value({nullptr, status});
            // Remove the shared future from the cache because it's
            // invalidated. An invalidated shared future is the one that
            // stores a nullptr.
            global_compiled_partition_cache.remove_if_invalidated(key);
            return status;
        } else {
            // Store the created compiled partition in the shared future
            // and notify the waiting threads.
            std::shared_ptr<impl::compiled_partition_t> new_cp(
                    new impl::compiled_partition_t(*this));
            new_cp->init(compiled_partition.first->pimpl_);
            assertm(new_cp->is_initialized(),
                    "Compiled partition is not initialized.");
            cp_promise.set_value({new_cp, status});

            // According to the doc of primitive cache, it says:
            //
            //   The key_t contains pointers to op_desc and attr objects
            //   that reside in pd. When primitive_t is created it copies
            //   the pd and hence contains a copy. Since the created
            //   primitive_t is stored in the cache with the corresponding
            //   key, the key must contain pointers to op_desc and attr that
            //   reside in the coppied pd in the primitive_t. Therefore the
            //   pointers in the key, which has already been put into the
            //   cache, must be updated.
            //
            // In the scenario of compiled partition cache, pointers the
            // key_t contains only engage the op_t, which are drived from
            // the shared_ptr<op_t> of a partition. As the shared_ptr<op_t>
            // also resides in a parition impl in cached compiled partition,
            // it's not necessary to update the key_t.
            //// global_compiled_partition_cache.update_entry(
            ////         key, &(new_cp->src_partition()), inputs, outputs);
        }
    }
    compiled_partition.second = is_from_cache;
    return status;
}

status_t dnnl_graph_compiled_partition::query_dynamic_outputs(
        const std::vector<impl::logical_tensor_t *> &out_lts,
        const std::vector<const impl::logical_tensor_t *> &in_lts,
        const impl::context_t *context) const {
    using ltw = impl::logical_tensor_wrapper_t;
    // check if provided input shapes are align with those passed through
    // compilation API
    if (!in_lts.empty()) {
        const auto &part_inputs = get_inputs();
        for (const auto &in : in_lts) {
            auto lid = in->id;
            auto found = std::find_if(part_inputs.begin(), part_inputs.end(),
                    [&lid](const impl::logical_tensor_t &e) {
                        return e.id == lid;
                    });
            // id is not found in partition's inputs
            if (found == part_inputs.end()) continue;
            auto found_ltw = ltw(*found);
            auto given_ltw = ltw(in);
            if (found_ltw.ndims() != given_ltw.ndims()) {
                assertm(false,
                        "ndims should be equal to original input for dynamic "
                        "shape inference.");
                return impl::status::invalid_shape;
            }

            for (int32_t i = 0; i < found_ltw.ndims(); ++i) {
                // other than dynamic dimension, those determinable
                // dimension values should be the same as original partition
                // input dimension values
                if (!ltw::is_dim_dynamic(found_ltw.dims()[i])
                        && found_ltw.dims()[i] != given_ltw.dims()[i]) {
                    assertm(false,
                            "dimension is either dynamic or equal to original "
                            "dim value.");
                    return impl::status::invalid_shape;
                }
            }
        }
    }

    return pimpl_->query_dynamic_outputs(out_lts, in_lts, context);
}

status_t dnnl_graph_compiled_partition::execute(const stream_t *astream,
        const std::vector<tensor_t> &inputs,
        const std::vector<tensor_t> &outputs) const {
    if (!astream || !astream->get_engine()->match(pimpl_->get_engine()))
        return status::invalid_arguments;

    if (astream->get_engine()->kind() == engine_kind::gpu) {
#ifdef DNNL_GRAPH_GPU_SYCL
        return execute_sycl(astream, inputs, outputs, {}, nullptr);
#else
        return status::runtime_error;
#endif
    } else {
#ifdef DNNL_GRAPH_CPU_SYCL
        return execute_sycl(astream, inputs, outputs, {}, nullptr);
#endif
    }

    const backend *backend = src_partition_.get_assigned_backend();
    if (!backend) return status::invalid_arguments;

    // Pre-process the given tensor. The pre-process includes
    // FIXME(xx) reduce overhead?
    // 1. decode backend id from the layout id and remove it
    std::vector<tensor_t> processed_inputs, processed_outputs;
    pre_process(processed_inputs, inputs, backend);
    pre_process(processed_outputs, outputs, backend);

    return pimpl_->execute(astream, processed_inputs, processed_outputs);
}

#ifdef DNNL_GRAPH_WITH_SYCL
status_t dnnl_graph_compiled_partition::execute_sycl(const stream_t *astream,
        const std::vector<tensor_t> &inputs,
        const std::vector<tensor_t> &outputs,
        const std::vector<::sycl::event> &sycl_deps,
        ::sycl::event *sycl_event) const {
    if (!astream || !astream->get_engine()->match(pimpl_->get_engine()))
        return status::invalid_arguments;

    status_t ret;

    const backend *backend = src_partition_.get_assigned_backend();
    if (!backend) return status::invalid_arguments;

    // Pre-process the given tensor. The pre-process includes
    // 1. decode backend id from the layout id and remove it
    std::vector<tensor_t> processed_inputs, processed_outputs;
    pre_process(processed_inputs, inputs, backend);
    pre_process(processed_outputs, outputs, backend);

    ret = pimpl_->execute_sycl(astream, processed_inputs, processed_outputs,
            sycl_deps, sycl_event);

    return ret;
}
#endif // DNNL_GRAPH_WITH_SYCL
