/*******************************************************************************
* Copyright 2021-2022 Intel Corporation
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

#ifndef BACKEND_FAKE_FAKE_PARTITION_IMPL_HPP
#define BACKEND_FAKE_FAKE_PARTITION_IMPL_HPP

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

#include "interface/backend.hpp"
#include "interface/partition.hpp"

#include "backend/fake/fake_backend.hpp"

namespace dnnl {
namespace graph {
namespace impl {
namespace fake_impl {

class fake_partition_impl_t : public impl::partition_impl_t {
    friend class fake_backend_t;

public:
    fake_partition_impl_t(impl::engine_kind_t engine_kind)
        : impl::partition_impl_t(engine_kind) {}

    ~fake_partition_impl_t() override = default;

    ///// The following are used only in backend for constructing object

    void init(const impl::op_t *aop) {
        fused_op_ = impl::utils::make_unique<impl::op_t>(aop->get_kind());
        fused_op_->merge_attributes(aop->get_attributes());
        add_tensors(aop);
        add_tensors_map(aop);
    }

    void add_op(const std::shared_ptr<op_t> &op) { ops_.emplace_back(op); }

    void add_op(const std::vector<std::shared_ptr<op_t>> &ops) {
        for (auto &op : ops) {
            add_op(op);
        }
    }

    void add_tensors(const impl::op_t *op) {
        for (size_t i = 0; i < op->num_inputs(); ++i) {
            inputs_.push_back(op->get_input_value(i)->get_logical_tensor());
        }
        for (size_t i = 0; i < op->num_outputs(); ++i) {
            outputs_.push_back(op->get_output_value(i)->get_logical_tensor());
        }
    }

    void add_tensors_map(const impl::op_t *aop) {
        for (auto kv : aop->get_input_tensor_map()) {
            inputs_map_[kv.second] = kv.first;
        }
        for (auto kv : aop->get_output_tensor_map()) {
            outputs_map_[kv.second] = kv.first;
        }
    }

    impl::logical_tensor_t *find_input(size_t id, size_t offset) {
        auto p = std::make_pair(id, offset);

        auto v = inputs_map_.find(p);
        if (v != inputs_map_.end()) {
            return &(inputs_.at(v->second));
        } else {
            return nullptr;
        }
    }

    impl::logical_tensor_t *find_output(size_t id, size_t offset) {
        auto p = std::make_pair(id, offset);

        auto v = outputs_map_.find(p);
        if (v != outputs_map_.end()) {
            return &(outputs_.at(v->second));
        } else {
            return nullptr;
        }
    }

    /////////////// the followings are the implementation of interface

    bool is_initialized() const override { return fused_op_ != nullptr; }

    std::shared_ptr<impl::partition_impl_t> clone() const override {
        auto ret = std::make_shared<fake_partition_impl_t>(get_engine_kind());
        ret->ops_ = impl::graph_t::deep_copy(ops_);
        ret->inputs_ = inputs_;
        ret->outputs_ = outputs_;
        ret->id_ = id_;
        ret->fused_op_ = std::make_shared<impl::op_t>(fused_op_->get_kind());
        ret->fused_op_->merge_attributes(fused_op_->get_attributes());
        ret->inputs_map_ = inputs_map_;
        ret->outputs_map_ = outputs_map_;
        return ret;
    }

    const impl::backend *get_assigned_backend() const override {
        return &fake_backend_t::get_singleton();
    }

    impl::status_t compile(impl::compiled_partition_t *compiled_partition,
            const std::vector<impl::logical_tensor_t> &inputs,
            const std::vector<impl::logical_tensor_t> &outputs,
            const impl::engine_t *g_engine,
            const impl::context_t *acontext) const override {
        UNUSED(compiled_partition);
        UNUSED(inputs);
        UNUSED(outputs);
        UNUSED(g_engine);
        UNUSED(acontext);
        return status::unimplemented;
    }

    impl::status_t infer_shape(
            std::vector<const impl::logical_tensor_t *> &inputs,
            std::vector<impl::logical_tensor_t *> &outputs) const override {
        UNUSED(inputs);
        UNUSED(outputs);
        return status::unimplemented;
    }

    impl::op_t *get_fused_op() const { return fused_op_.get(); }

private:
    // // Fused op. Currently, only one op here
    std::shared_ptr<impl::op_t> fused_op_ {nullptr};

    // Map from (op id, op input offset) -> partition input index
    std::unordered_map<std::pair<size_t, size_t>, size_t> inputs_map_;

    // Map from (op id, op output offset) -> partition output index
    std::unordered_map<std::pair<size_t, size_t>, size_t> outputs_map_;
};

} // namespace fake_impl
} // namespace impl
} // namespace graph
} // namespace dnnl

#endif
