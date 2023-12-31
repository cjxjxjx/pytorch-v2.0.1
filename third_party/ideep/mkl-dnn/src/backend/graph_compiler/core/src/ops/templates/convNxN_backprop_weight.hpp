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

#ifndef BACKEND_GRAPH_COMPILER_CORE_SRC_OPS_TEMPLATES_CONVNXN_BACKPROP_WEIGHT_HPP
#define BACKEND_GRAPH_COMPILER_CORE_SRC_OPS_TEMPLATES_CONVNXN_BACKPROP_WEIGHT_HPP

#include <memory>
#include <tuple>
#include <vector>
#include "conv_bwd.hpp"
#include <ops/body_generator.hpp>
namespace sc {

namespace ops {

class gen_convNxN_backprop_weight
  : public body_generator_t<conv_bwd_weight_config_t> {
public:
  sc_dims stride_;
  sc_dims padding_;
  struct op_params_t {
    static constexpr int in_data = 0;
    static constexpr int in_fwd_output = 1;
    static constexpr int out_del_weight = 0;
  };
  enum generator_type_t { REDUCE_N, REDUCE_W, UNDEF };
  generator_type_t type_;
  using parent = body_generator_t<conv_bwd_weight_config_t>;
  using parent::generate;

  gen_convNxN_backprop_weight(sc_op *owner, const sc_dims &stride,
    const sc_dims &padding, std::vector<logical_tensor_t> &&ins,
    std::vector<logical_tensor_t> &&outs,
    generator_type_t type = generator_type_t::REDUCE_N);

  float get_gflop() const override;

  const sc_dims &get_data_dims() const {
    return in_tensors_[op_params_t::in_data].get_plain_dims();
  }
  const sc_dims &get_grad_dims() const {
    return in_tensors_[op_params_t::in_fwd_output].get_plain_dims();
  }
  const sc_dims &get_output_dims() const {
    return out_tensors_[op_params_t::out_del_weight].get_plain_dims();
  }
  sc_data_type_t get_dtype() const { return in_tensors_[0].dtype_; }

  bool generate(context_ptr ctx, const conv_bwd_weight_config_t &config,
    fusion_manager *fusion, const std::vector<expr> &inputs,
    const std::vector<expr> &outputs,
    std::vector<for_loop> &loops) const override;

  bool generate_reduce_N(const context_ptr &ctx,
    const conv_bwd_weight_config_t &config, fusion_manager *fusion,
    const std::vector<expr> &inputs, const std::vector<expr> &outputs,
    std::vector<for_loop> &loops) const;

  bool generate_reduce_W(const context_ptr &ctx,
    const conv_bwd_weight_config_t &config, fusion_manager *fusion,
    const std::vector<expr> &inputs, const std::vector<expr> &outputs,
    std::vector<for_loop> &loops) const;
  config_ptr get_default_config(context_ptr ctx) const override;

  void schedule_loops(context_ptr ctx, const conv_bwd_weight_config_t &config,
    stmt body, std::vector<for_loop> &fors) const override;

private:
  int ndims_ = 0;
};
} // namespace ops

} // namespace sc

#endif
