/*******************************************************************************
 * Copyright 2022 Intel Corporation
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
#ifndef BACKEND_GRAPH_COMPILER_CORE_SRC_OPS_REDUCE_MEAN_HPP
#define BACKEND_GRAPH_COMPILER_CORE_SRC_OPS_REDUCE_MEAN_HPP

#include <memory>
#include <vector>
#include "fusible/reduce.hpp"
#include <compiler/ir/graph/graph_op.hpp>

namespace sc {

class reduce_mean_op_t : public graph_op_t, public op_traits::auto_copyable_t {
public:
    reduce_mean_op_t(graph_tensor_ptr v, const std::vector<int> &rd_axis,
            bool keep_dims = false);
    reduce_mean_op_t(const std::vector<graph_tensor_ptr> &ins,
            const std::vector<graph_tensor_ptr> &outs, const any_map_t &attrs);
    void get_graph_impl(std::shared_ptr<sc_graph_t> &graph) override;
    void query_format(context_ptr ctx,
            std::vector<std::vector<format_stride_pair>> &supported_ins,
            std::vector<std::vector<format_stride_pair>> &supported_outs)
            override;
};

} // namespace sc

#endif
