#-------------------------------------------------------------------------------
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#-------------------------------------------------------------------------------

set(test_files
        ${CMAKE_CURRENT_SOURCE_DIR}/test_db.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_db_mysql.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_engine.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_mem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_meta.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_meta_mysql.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_misc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_search.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp)

cuda_add_executable(test_db
        ${common_files}
        ${test_files}
        )

target_link_libraries(test_db
        knowhere
        ${unittest_libs})

install(TARGETS test_db DESTINATION unittest)



