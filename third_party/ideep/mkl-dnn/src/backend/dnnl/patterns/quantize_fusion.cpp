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

#include "backend/dnnl/kernels/quantize.hpp"
#include "backend/dnnl/patterns/fusions.hpp"
#include "backend/dnnl/patterns/transformation_pattern.hpp"
#include "utils/pm/pbuilder.hpp"

namespace dnnl {
namespace graph {
namespace impl {
namespace dnnl_impl {
namespace pattern {

/*!
 * \brief This provides quantize fusion.
 *        The process includes follow steps:
 *          1. look for fusion pattern on the graph
 *          2. If found, verify if this transformation is safe / correct
 *          3. replace the pattern with a fused op, update the graph
 */

namespace pm = impl::utils::pm;
using in_edges_t = pm::in_edges_t;
using pb_graph_t = impl::utils::pm::pb_graph_t;
using FCreatePattern = impl::pass::FCreatePattern;

namespace {
bool check_inputs_all_bf16(op_t *op) {
    for (size_t i = 0; i < op->num_inputs(); ++i) {
        logical_tensor_t iport = op->get_input_value(i)->get_logical_tensor();
        if (iport.data_type != impl::data_type::bf16) return false;
    }
    return true;
}
} // namespace

DNNL_BACKEND_REGISTER_PATTERN_DEF_BEGIN(quantize_fusion)

DNNL_BACKEND_REGISTER_TRANSFORMATION_PATTERN(dnnl, typecast_quantize_fusion)
        .set_priority(8.1f)
        .set_kind(impl::partition_kind::misc_quantized_post_ops)
        .set_attr<FCreatePattern>("FCreatePattern",
                [](const std::shared_ptr<pb_graph_t> &pgraph) -> void {
                    pm::pb_op_t *typecast
                            = pgraph->append_op(impl::op_kind::TypeCast);
                    // check it is a bf16->f32 typecast
                    typecast->append_decision_function(check_inputs_all_bf16);

                    pgraph->append_op(impl::op_kind::Quantize,
                            in_edges_t {in_edge(0, typecast, 0)});
                })
        .set_attr<FCreateKernel>("FCreateKernel", []() -> kernel_ptr {
            return std::make_shared<quantize_dequantize_t>();
        });

DNNL_BACKEND_REGISTER_PATTERN_DEF_END

} // namespace pattern
} // namespace dnnl_impl
} // namespace impl
} // namespace graph
} // namespace dnnl
