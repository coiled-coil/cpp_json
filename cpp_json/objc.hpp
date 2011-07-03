//
//  Copyright 2011 Kaoru Yanase. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#pragma once
#import <Foundation/Foundation.h>
#include "cjson_wrapper.hpp"

namespace cpp_json {

    // NSString *
    template <>
    class cjson_cast_impl<NSString *>
    {
    public:
        static boost::optional<NSString*> get(cJSON *value)
        {
            if (value) {
                if (value->type == cJSON_String) {
                    return [NSString stringWithUTF8String:value->valuestring];
                }
            }
            return boost::optional<NSString*>();
        }
    };

    cjson_document parse(NSString *str)
    {
        return cjson_document(cJSON_Parse([str UTF8String]));
    }
} // namespace cpp_json
