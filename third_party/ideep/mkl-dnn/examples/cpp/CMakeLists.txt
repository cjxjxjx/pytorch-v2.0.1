#===============================================================================
# Copyright 2020-2022 Intel Corporation
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

if(NOT DNNL_GRAPH_BUILD_EXAMPLES)
    return()
endif()

enable_testing()

# walkaround for icx cannot init constexpr with dllimport symbols(detail::handle)
# also, suppose it's unnecessary for users to link the library with such flag
if (DNNL_GRAPH_LIBRARY_TYPE STREQUAL "SHARED" OR DNNL_GRAPH_LIBRARY_TYPE STREQUAL "SDL")
    remove_definitions(-DDNNL_GRAPH_DLL)
endif()

# propagate common compilation flags
append(CMAKE_C_FLAGS "${DNNL_GRAPH_COMMON_COMPILATION_FLAGS}")
append(CMAKE_CXX_FLAGS "${DNNL_GRAPH_COMMON_COMPILATION_FLAGS}")

# propagate EXAMPLE specific flags
append(CMAKE_C_FLAGS "${CMAKE_EXAMPLE_CCXX_FLAGS}")
append(CMAKE_CXX_FLAGS "${CMAKE_EXAMPLE_CCXX_FLAGS}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    append_if(WIN32 CMAKE_CXX_FLAGS "/fp:precise")
    append_if(UNIX  CMAKE_CXX_FLAGS "-fp-model precise")
endif()

# Register demo
function(register_example name srcs)
    if(TARGET ${name})
        message(STATUS "target with the same name already exists")
        return()
    endif()

    add_executable(${name} ${srcs})
    target_include_directories(${name}
        PRIVATE
        ${PROJECT_SOURCE_DIR}/examples/cpp/include
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/third_party/oneDNN/include
        ${PROJECT_SOURCE_DIR}/third_party/oneDNN/include
    )

    target_link_libraries(${name} 
        LINK_PRIVATE
        dnnl_graph
        ${DNNL_GRAPH_COMMON_LINK_FLAGS}
        ${DNNL_GRAPH_SYCL_LINK_FLAGS}
    )
endfunction()

file(GLOB sources *.cpp)

foreach(src ${sources})
    file(RELATIVE_PATH src_rel_path ${CMAKE_CURRENT_SOURCE_DIR} ${src})
    string(REGEX REPLACE "[/_\\.]" "-" example_name ${src_rel_path})

    set(cpu_rt_pattern "(SEQ|OMP|TBB)")
    set(sycl_rt_pattern "(DPCPP)")
    if(DNNL_GRAPH_CPU_RUNTIME MATCHES ${cpu_rt_pattern})
	# Adding examples for OMP CPU
        if(${example_name} MATCHES "cpu-*")
            register_example(${example_name} ${src})
            add_test("${example_name}" "${example_name}" cpu)
            maybe_configure_windows_test("${example_name}" TEST)
        endif()
    endif()
    if(DNNL_GRAPH_CPU_RUNTIME MATCHES ${sycl_rt_pattern})
	# Adding examples for SYCL CPU
        if(${example_name} MATCHES "sycl-*")
            register_example(${example_name} ${src})
            add_test("${example_name}-cpu" "${example_name}" cpu)
            maybe_configure_windows_test("${example_name}-cpu" TEST)
        endif()
    endif()
    if(DNNL_GRAPH_GPU_RUNTIME MATCHES ${sycl_rt_pattern})
	# Adding examples for SYCL GPU
        if(${example_name} MATCHES "gpu-*")
            register_example(${example_name} ${src})
            add_test("${example_name}" "${example_name}" gpu)
            maybe_configure_windows_test("${example_name}" TEST)
        endif()
	# Adding examples for SYCL GPU
        if(${example_name} MATCHES "sycl-*")
            register_example(${example_name} ${src})
            add_test("${example_name}-gpu" "${example_name}" gpu)
            maybe_configure_windows_test("${example_name}-gpu" TEST)
        endif()
    endif()
endforeach()
