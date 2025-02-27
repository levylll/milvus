// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#pragma once

#include "utils/Status.h"

#include <string>
#include <vector>

namespace milvus {
namespace server {

class StringHelpFunctions {
 private:
    StringHelpFunctions() = default;

 public:
    static void
    TrimStringBlank(std::string& string);

    static void
    TrimStringQuote(std::string& string, const std::string& qoute);

    // split string by delimeter ','
    // a,b,c            a | b | c
    // a,b,             a | b |
    // ,b,c               | b | c
    // ,b,                | b |
    // ,,                 |   |
    // a                    a
    static void
    SplitStringByDelimeter(const std::string& str, const std::string& delimeter, std::vector<std::string>& result);

    static void
    MergeStringWithDelimeter(const std::vector<std::string>& strs, const std::string& delimeter, std::string& result);

    // assume the table has two columns, quote='\"', delimeter=','
    //  a,b             a | b
    //  "aa,gg,yy",b    aa,gg,yy | b
    //  aa"dd,rr"kk,pp  aadd,rrkk | pp
    //  "aa,bb"         aa,bb
    //  55,1122\"aa,bb\",yyy,\"kkk\"    55 | 1122aa,bb | yyy | kkk
    //  "55,1122"aa,bb",yyy,"kkk"   illegal
    static Status
    SplitStringByQuote(const std::string& str, const std::string& delimeter, const std::string& quote,
                       std::vector<std::string>& result);
};

}  // namespace server
}  // namespace milvus
