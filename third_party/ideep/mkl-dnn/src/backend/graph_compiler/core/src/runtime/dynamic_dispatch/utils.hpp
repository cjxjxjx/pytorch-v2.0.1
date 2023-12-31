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
#ifndef BACKEND_GRAPH_COMPILER_CORE_SRC_RUNTIME_DYNAMIC_DISPATCH_UTILS_HPP
#define BACKEND_GRAPH_COMPILER_CORE_SRC_RUNTIME_DYNAMIC_DISPATCH_UTILS_HPP

#include <runtime/dynamic_dispatch/dynamic_tensor.hpp>

namespace sc {
namespace runtime {
/**
 * @brief Get the dynamic config single block from the plain dynamic dimension
 *
 * @param in the dynamic dimension
 * @param is_batch default false, candidates are [16, 32, 64], if true,
 * candidates are [2, 4, 8, 16, 32, 64].
 * @return the selected block config
 */
int get_dyn_cfg_single(int in, bool is_batch = false);

void deep_copy_dynamic_tensor(
        dynamic_tensor_t *out, const dynamic_tensor_t *in);

uint64_t calculate_blocking_dims(void *placeholder, uint64_t *format);

} // namespace runtime
} // namespace sc

#endif
