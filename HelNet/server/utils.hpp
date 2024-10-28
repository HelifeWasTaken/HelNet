/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include <string>
#include "HelNet/logger.hpp" // Includes the fmt::format function

namespace hl
{
namespace net
{
namespace utils
{
    template<class ProtocolEndpoint>
    static std::string endpoint_to_string(const ProtocolEndpoint& endpoint)
    {
        return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
    }

    template<typename K, typename V>
    class back_and_forth_unordered_map
    {
    private:
        std::unordered_map<K, V> forward;
        std::unordered_map<V, K> backward;

    public:
        using forward_iterator = typename std::unordered_map<K, V>::iterator;
        using backward_iterator = typename std::unordered_map<V, K>::iterator;
        using const_forward_iterator = typename std::unordered_map<K, V>::const_iterator;
        using const_backward_iterator = typename std::unordered_map<V, K>::const_iterator;

        back_and_forth_unordered_map()
            : forward()
            , backward()
        {}

        ~back_and_forth_unordered_map() = default;

        back_and_forth_unordered_map(const back_and_forth_unordered_map &) = default;
        back_and_forth_unordered_map &operator=(const back_and_forth_unordered_map &) = default;
        back_and_forth_unordered_map(back_and_forth_unordered_map &&) = default;
        back_and_forth_unordered_map &operator=(back_and_forth_unordered_map &&) = default;

        void insert(const K &key, const V &value)
        {
            forward[key] = value;
            backward[value] = key;
        }

        void erase(const K &key)
        {
            backward.erase(forward[key]);
            forward.erase(key);
        }

        void erase(const V &value)
        {
            forward.erase(backward[value]);
            backward.erase(value);
        }

        void erase(const_forward_iterator it)
        {
            backward.erase(it->second);
            forward.erase(it);
        }

        void erase(const_backward_iterator it)
        {
            forward.erase(it->second);
            backward.erase(it);
        }

        V &operator[](const K &key)
        {
            return forward[key];
        }

        K &operator[](const V &value)
        {
            return backward[value];
        }

        bool contains(const K &key) const
        {
            return forward.find(key) != forward.end();
        }

        bool contains(const V &value) const
        {
            return backward.find(value) != backward.end();
        }

        size_t size() const
        {
            return forward.size();
        }

        bool empty() const
        {
            return forward.empty();
        }

        void clear()
        {
            forward.clear();
            backward.clear();
        }

        forward_iterator find_forward(const K &key)
        {
            return forward.find(key);
        }

        backward_iterator find_backward(const V &value)
        {
            return backward.find(value);
        }

        const_forward_iterator find_forward(const K &key) const
        {
            return forward.find(key);
        }

        const_backward_iterator find_backward(const V &value) const
        {
            return backward.find(value);
        }

        forward_iterator begin_forward()
        {
            return forward.begin();
        }

        forward_iterator end_forward()
        {
            return forward.end();
        }

        const_forward_iterator begin_forward() const
        {
            return forward.begin();
        }

        const_forward_iterator end_forward() const
        {
            return forward.end();
        }

        backward_iterator begin_backward()
        {
            return backward.begin();
        }

        backward_iterator end_backward()
        {
            return backward.end();
        }

        const_backward_iterator begin_backward() const
        {
            return backward.begin();
        }

        const_backward_iterator end_backward() const
        {
            return backward.end();
        }
    };
}
}
}