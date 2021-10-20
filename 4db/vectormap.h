#pragma once

#include <assert.h>

#include <unordered_map>
#include <vector>

namespace seadb
{
    template <typename K, typename V>
    struct kvpair
    {
        kvpair(const K& k, const V& v)
            : key(k)
            , value(v)
        {}

        K key;
        V value;
    };

    template <typename K, typename V>
    class vectormap
    {
    public:
        const std::vector<kvpair<K, V>>& vec() const
        {
            return m_vec;
        }

        const std::unordered_map<K, V>& map() const
        {
            return m_map;
        }

        /* FORNOW - Needed?
        std::vector<K> keys() const
        {
            std::vector<K> ks(m_vec.size());
            for (const auto& kvp : m_vec)
                ks.push_back(kvp.Key);
            return ks;
        }

        std::vector<V> values() const
        {
            std::vector<V> vs(m_vec.size());
            for (const auto& kvp : m_vec)
                vs.push_back(kvp.Value);
            return vs;
        }

        V get(const K& key) const
        {
            assert(contains(key));
            return m_map[key];
        }
        */

        size_t size() const
        {
            return m_vec.size();
        }

        bool contains(const K& key) const
        {
            return m_map.find(key) != m_map.end();
        }

        void insert(K key, V val)
        {
            m_map.insert(key, val);
            m_vec.emplace_back(key, val);
        }

        bool tryget(const K& key, V& val) const
        {
            auto it = m_map.find(key);
            if (it == m_map.end())
                return false;

            val = it.second;
            return true;
        }

    private:
        std::vector<kvpair<K, V>> m_vec;
        std::unordered_map<K, V> m_map;
    };
}
