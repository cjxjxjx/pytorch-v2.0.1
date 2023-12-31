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

#include <vector>
#include <gtest/gtest.h>

#include "oneapi/dnnl/dnnl_graph.hpp"

#ifndef DNNL_GRAPH_CPU_SYCL
TEST(APIEngine, SimpleCreate) {
    using namespace dnnl::graph;
    allocator alloc {};
    engine e {engine::kind::cpu, 0, alloc};

    ASSERT_EQ(e.get_kind(), engine::kind::cpu);
}
#endif
