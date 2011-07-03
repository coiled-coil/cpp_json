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
#include <cJSON.h>
#include "detail/safe_bool.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/iterator_adaptors.hpp>
#include <boost/optional.hpp>
#include <stdexcept>
#include <string>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>

namespace cpp_json {

    struct json_parse_error : public std::runtime_error
    {
        /** Takes a character string describing the error.  */
        explicit json_parse_error(const std::string& msg)
        :   std::runtime_error(msg)
        {
        }
    };

    // Customization point for library developers
    template <typename ResultType>
    class cjson_cast_impl
    {
    public:
        static boost::optional<ResultType> get(cJSON *json)
        {
            return ResultType::__UNDEFINED_TYPE_ERROR__;
        }
    };

    // Public interface for users
    template <typename ResultType>
    boost::optional<ResultType> cjson_cast(cJSON *json)
    {
        return cjson_cast_impl<ResultType>::get(json);
    }

    // --- Some specializations for well-known types

    // const char *
    template <>
    class cjson_cast_impl<const char *>
    {
    public:
        static boost::optional<const char*> get(cJSON *value)
        {
            if (value) {
                if (value->type == cJSON_String) {
                    return value->valuestring;
                }
            }
            return boost::optional<const char*>();
        }
    };

    // int
    template <>
    class cjson_cast_impl<int>
    {
    public:
        static boost::optional<int> get(cJSON *value)
        {
            if (value) {
                if (value->type == cJSON_Number) {
                    return value->valueint;
                }
            }
            return boost::optional<int>();
        }
    };

    template <typename ValueType, typename BaseType>
    class cjson_iterator;

    class cjson_wrapper
    : public detail::safe_bool<cjson_wrapper>
    {
    public:
        typedef cjson_iterator<cjson_wrapper, cJSON> iterator;
        typedef cjson_iterator<cjson_wrapper const, cJSON> const_iterator;
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        const_iterator cbegin() const;
        const_iterator cend() const;

    public:
        explicit cjson_wrapper(cJSON *json)
        :   json_(json)
        {
        }

        cjson_wrapper next() const
        {
            return cjson_wrapper(json_->next);
        }

        template <typename RetType>
        RetType as() const
        {
            if (boost::optional<RetType> v = cjson_cast<RetType>(json_)) {
                return *v;
            }
            else {
                throw json_parse_error("JSON parse error");
            }
        }

        template <typename RetType>
        RetType get_or_default(RetType const& default_value) const
        {
            if (boost::optional<RetType> v = cjson_cast<RetType>(json_)) {
                return *v;
            }
            else {
                return default_value;
            }
        }

        template <typename RetType>
        boost::optional<RetType> get_optional() const
        {
            return cjson_cast<RetType>(json_);
        }

        size_t size() const
        {
            return cJSON_GetArraySize(json_);
        }

        cjson_wrapper operator[](size_t index) const
        {
            return cjson_wrapper(cJSON_GetArrayItem(json_, index));
        }

        cjson_wrapper operator[](const char *key) const
        {
            return cjson_wrapper(cJSON_GetObjectItem(json_, key));
        }

        cJSON *raw_pointer()
        {
            return json_;
        }

        const cJSON *raw_pointer() const
        {
            return json_;
        }

    public:
        // Implementation for safe_bool CRTP
        bool boolean_test() const
        {
            return json_ != 0 && json_->type != cJSON_NULL && json_->type != cJSON_False;
        }

    private:
        cJSON *json_;
    };

    template <typename ValueType, typename BaseType>
    class cjson_iterator
    : public boost::iterator_facade<
                cjson_iterator<ValueType, BaseType>,
                ValueType,                      // value type
                boost::forward_traversal_tag,   // iterator traits
                ValueType                       // Reference
    >
    {
    private:
        struct enabler {};

    public:
        cjson_iterator()
        : node_()
        , boost::iterator_facade<cjson_iterator<ValueType, BaseType>,
                                 ValueType,
                                 boost::forward_traversal_tag,
                                 ValueType>()
        {
        }

        explicit cjson_iterator(BaseType *p)
        : node_(p)
        , boost::iterator_facade<cjson_iterator<ValueType, BaseType>,
                                 ValueType,
                                 boost::forward_traversal_tag,
                                 ValueType>()
        {
        }

        BaseType *_internal_pointer() { return node_; }
        BaseType const *_internal_pointer() const { return node_; }

        template <typename OtherValue, typename OtherBase>
        cjson_iterator(cjson_iterator<OtherValue, OtherBase> const& other,
                       typename boost::enable_if<boost::is_convertible<OtherBase*,BaseType*>,enabler>::type = enabler())
        : node_(other._internal_pointer())
        , boost::iterator_facade<cjson_iterator<ValueType, BaseType>,
                                 ValueType,
                                 boost::forward_traversal_tag,
                                 ValueType>()
        {
        }

    private:
        friend class boost::iterator_core_access;

        template <typename OtherValue, typename OtherBase>
        bool equal(cjson_iterator<OtherValue, OtherBase> const& other) const
        {
            return this->node_ == other._internal_pointer();
        }

        void increment()
        {
            node_ = node_->next;
        }

        ValueType dereference() const
        {
            return cjson_wrapper(node_);
        }

    private:
        BaseType *node_;
    };

    cjson_wrapper::iterator cjson_wrapper::begin()
    {
        return iterator(json_->child);
    }

    cjson_wrapper::iterator cjson_wrapper::end()
    {
        return iterator(NULL);
    }

    cjson_wrapper::const_iterator cjson_wrapper::cbegin() const
    {
        return const_iterator(json_->child);
    }

    cjson_wrapper::const_iterator cjson_wrapper::cend() const
    {
        return const_iterator(NULL);
    }

    cjson_wrapper::const_iterator cjson_wrapper::begin() const
    {
        return cbegin();
    }

    cjson_wrapper::const_iterator cjson_wrapper::end() const
    {
        return cend();
    }

    class cjson_document : public cjson_wrapper
    {
        boost::shared_ptr<cJSON> json_;
    public:
        explicit cjson_document(cJSON *json)
        :   json_(json, cJSON_Delete)
        ,   cjson_wrapper(json)
        {
        }
    };

    cjson_document parse(const char *str)
    {
        return cjson_document(cJSON_Parse(str));
    }
} // namespace cpp_json
