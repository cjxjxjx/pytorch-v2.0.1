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
#ifndef BACKEND_GRAPH_COMPILER_CORE_SRC_COMPILER_IR_TRANSFORM_PARALLEL_WORKLOAD_ATTR_HPP
#define BACKEND_GRAPH_COMPILER_CORE_SRC_COMPILER_IR_TRANSFORM_PARALLEL_WORKLOAD_ATTR_HPP
#include <stddef.h>

namespace sc {

namespace parallel_workload {
static const size_t read_weight = 1UL;
static const size_t write_weight = 1UL;
static constexpr const char *attr_workload_number = "workload_number";
} // namespace parallel_workload

} // namespace sc

#endif
