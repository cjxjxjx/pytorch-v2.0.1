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

#include <vector>
#include <gtest/gtest.h>

#include "cpp/unit/utils.hpp"

#include "interface/backend.hpp"
#include "interface/logical_tensor.hpp"

#include "backend/dnnl/dnnl_backend.hpp"
#include "backend/fake/fake_backend.hpp"

namespace impl = dnnl::graph::impl;
namespace utils = dnnl::graph::tests::unit::utils;

TEST(Backend, CompareLogicalTensor) {
    impl::backend &bkd = impl::fake_impl::fake_backend_t::get_singleton();

    impl::logical_tensor_t lt1 = utils::logical_tensor_init(
            0, {1, 2, 2, 1}, impl::data_type::f32, impl::layout_type::undef);
    impl::logical_tensor_t lt2 = utils::logical_tensor_init(
            0, {1, 2, 2, 1}, impl::data_type::f32, impl::layout_type::undef);
    impl::logical_tensor_t lt3 = utils::logical_tensor_init(
            0, {1, 2, 2, 1}, impl::data_type::f32, impl::layout_type::opaque);
    impl::logical_tensor_t lt4 = utils::logical_tensor_init(
            1, {1, 2, 2, 1}, impl::data_type::f32, impl::layout_type::undef);
    impl::logical_tensor_t lt5 = utils::logical_tensor_init(
            0, {1, 2, 2, 1}, impl::data_type::f32, impl::layout_type::strided);
    impl::logical_tensor_t lt6 = utils::logical_tensor_init(
            0, {1, 2, 2, 1}, impl::data_type::f32, impl::layout_type::strided);
    impl::logical_tensor_t lt7 = utils::logical_tensor_init(
            1, {1, 2, 2, 1}, impl::data_type::f32, impl::layout_type::opaque);

    ASSERT_EQ(bkd.compare_logical_tensor(lt1, lt2), true);
    ASSERT_EQ(bkd.compare_logical_tensor(lt1, lt4), true);
    ASSERT_EQ(bkd.compare_logical_tensor(lt1, lt3), false);
    ASSERT_EQ(bkd.compare_logical_tensor(lt5, lt6), true);
    ASSERT_EQ(bkd.compare_logical_tensor(lt3, lt7), true);
}

TEST(Backend, RegisterBackend) {
    auto &registry = impl::backend_registry_t::get_singleton();
    auto bkds = registry.get_registered_backends();
    EXPECT_THROW(registry.register_backend(bkds[0]), std::runtime_error);
}
