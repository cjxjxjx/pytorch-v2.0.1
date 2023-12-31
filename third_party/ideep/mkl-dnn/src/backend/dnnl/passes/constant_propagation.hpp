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
#ifndef BACKEND_DNNL_PASSES_CONSTANT_PROPAGATION_HPP
#define BACKEND_DNNL_PASSES_CONSTANT_PROPAGATION_HPP

#include <memory>

#include "interface/c_types_map.hpp"

#include "backend/dnnl/subgraph.hpp"

namespace dnnl {
namespace graph {
namespace impl {
namespace dnnl_impl {

impl::status_t constant_propagation(std::shared_ptr<subgraph_t> &sg);

} // namespace dnnl_impl
} // namespace impl
} // namespace graph
} // namespace dnnl

#endif
