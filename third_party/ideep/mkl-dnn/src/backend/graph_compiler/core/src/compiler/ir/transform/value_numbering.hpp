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
#ifndef BACKEND_GRAPH_COMPILER_CORE_SRC_COMPILER_IR_TRANSFORM_VALUE_NUMBERING_HPP
#define BACKEND_GRAPH_COMPILER_CORE_SRC_COMPILER_IR_TRANSFORM_VALUE_NUMBERING_HPP

#include <compiler/ir/function_pass.hpp>

namespace sc {

/**
 * Value numbering for SSA form IR. It will fold SSA values into one if the
 * expressions looks the same and are not related to side-effects. It will also
 * do copy propagation and constant propagation of SSA. The pass also oberserves
 * commutative law and considers (a+b) and (b+a) as same expr.
 * */
class value_numbering_t : public function_pass_t {
public:
    func_c operator()(func_c f) override;
    SC_DECL_PASS_INFO_FUNC();
};

} // namespace sc

#endif
