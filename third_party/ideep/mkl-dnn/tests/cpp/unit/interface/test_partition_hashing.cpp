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
#include <thread>

#include "gtest/gtest.h"

#include "interface/c_types_map.hpp"
#include "interface/partition_hashing.hpp"
#include "interface/shape_infer.hpp"

#include "cpp/unit/unit_test_common.hpp"

namespace impl = dnnl::graph::impl;

TEST(PartitionHashing, ThreadId) {
    impl::engine_t &engine = get_engine();
    size_t id = 10056;
    impl::partition_hashing::key_t key {id, engine.kind(), {}, {}, {}};
    ASSERT_EQ(std::this_thread::get_id(), key.thread_id());
}

TEST(PartitionHashing, GetArrayHash) {
    size_t seed = 10000;
    const size_t num = 3;
    float arr[num] {1.0f, 2.0f, 3.0f};
    EXPECT_NO_FATAL_FAILURE(
            impl::partition_hashing::get_array_hash(seed, arr, num));
}

TEST(PartitionHashing, GetOpHash) {
    impl::op_t op {0, impl::op_kind::Wildcard, "wildcard"};
    ASSERT_NO_THROW(impl::partition_hashing::get_op_hash(op));
}
