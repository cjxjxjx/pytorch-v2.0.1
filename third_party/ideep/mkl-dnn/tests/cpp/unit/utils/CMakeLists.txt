#===============================================================================
# Copyright 2022 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#===============================================================================

set(OBJ_LIB ${LIB_NAME}_utils_unit_test)

add_library(${OBJ_LIB} OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/test_allocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_attribute_value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_compatible.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_debug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_pattern_matcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_verbose.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_json.cpp
)

set_property(GLOBAL APPEND PROPERTY ${LIB_NAME}_UNIT_TEST_DEPS $<TARGET_OBJECTS:${OBJ_LIB}>)
