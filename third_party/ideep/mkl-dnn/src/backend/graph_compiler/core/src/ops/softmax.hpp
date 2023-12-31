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
#ifndef BACKEND_GRAPH_COMPILER_CORE_SRC_OPS_SOFTMAX_HPP
#define BACKEND_GRAPH_COMPILER_CORE_SRC_OPS_SOFTMAX_HPP

#include <memory>
#include <vector>
#include <compiler/ir/graph/graph_op.hpp>

namespace sc {
namespace ops {
/**
 * The softmax operator: exp(x)/sum(exp(x))
 * Inputs:
 *  - A single tensor
 * Outputs:
 *  - The result tensor
 * Attrs:
 *  - axis: vector<int> - The reduce axis for the "sum", see "reduce" op
 * */
class softmax_op : public graph_op_t, public op_traits::auto_copyable_t {
public:
    softmax_op(const std::vector<graph_tensor_ptr> &ins,
            const std::vector<graph_tensor_ptr> &outs, const any_map_t &attrs);
    void get_graph_impl(std::shared_ptr<sc_graph_t> &graph) override;
    void query_format(context_ptr ctx,
            std::vector<std::vector<format_stride_pair>> &supported_ins,
            std::vector<std::vector<format_stride_pair>> &supported_outs)
            override;
};
} // namespace ops
} // namespace sc

#endif
