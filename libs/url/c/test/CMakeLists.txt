# ------------------------------ tabstop = 4 ----------------------------------
#
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2024 Comcast Cable Communications Management, LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
#
# ------------------------------ tabstop = 4 ----------------------------------

#--------------------------------------------------------------------------------------
#
# CMake definition for the fcore 'types library unit tests'
# Should not be utilized directly, but imported from our parent
# file via the 'add_subdirectory()' routine.
#
#--------------------------------------------------------------------------------------

# setup a unit test, but such that it is NOT part of the 'make all' sequence.
# allows us to only build/run unit tests on the platforms we can (meaning
# not during a cross-compile session).  should be build/ran as part of
# the 'make unitTest' command

# setup include paths
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CROSS_OUTPUT}/include         # 3rd Party
)

# define source files (using wildcard)
file(GLOB SOURCES "src/*.c")
list(FILTER SOURCES EXCLUDE REGEX ".*urlHelperTest.c$")

# build as an executable, but NOT part of the "make all" sequence
add_executable(testUrlHelperLib EXCLUDE_FROM_ALL ${SOURCES})

# add library dependencies for this test-binary
target_link_libraries(testUrlHelperLib xhUrlHelper curl cmocka)

# run as a 'test', however need to force the LD_LIBRARY_PATH since
# the RPATH will not be set (as this was not 'installed' via make)
add_test(testUrlHelperLib ${EXECUTABLE_OUTPUT_PATH}/testUrlHelperLib)
if (CONFIG_OS_DARWIN)
    set_property(TEST testUrlHelperLib PROPERTY ENVIRONMENT "DYLD_LIBRARY_PATH=${CROSS_OUTPUT}/lib:${CMAKE_INSTALL_PREFIX}/lib")
else()
    set_property(TEST testUrlHelperLib PROPERTY ENVIRONMENT "LD_LIBRARY_PATH=${CROSS_OUTPUT}/lib:${CMAKE_INSTALL_PREFIX}/lib")
endif()

# add dependency on the custom target 'unitTest' so it will be
# built/ran when specifically asked to
add_dependencies(unitTest testUrlHelperLib)

