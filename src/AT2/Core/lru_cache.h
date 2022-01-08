#pragma once

#include <unordered_map>
#include <list>
#include <cassert>

template <typename Key, typename Value>
class lru_cache
{
public:
    using key_type = Key;
    using value_type = std::pair<key_type, Value>;

    explicit lru_cache(size_t capacity) : m_index {capacity * 2}, m_capacity {capacity} {}

	//TODO: add heterogenous lookup, emplace, etc
    //TODO: make allocator friendly

    template <typename ValueFactoryFunc, typename ValueDroppedFunc>
    void put(const key_type& key, ValueFactoryFunc&& valueFactory, ValueDroppedFunc&& valueDroppedCallback)
	{
    	const auto [iterator, isNew] = m_index.emplace(key, ListIterator{});
        if (isNew)
        {
            assert(m_cache.size() <= m_capacity);
            if (m_cache.size() == m_capacity)
            {
                std::forward<ValueDroppedFunc>(valueDroppedCallback)(std::move(m_cache.back()));

                m_index.erase(m_cache.back().first);
                m_cache.pop_back(); //TODO: could be just rewritten instead of reallocation
            }

            m_cache.emplace_front(std::piecewise_construct, std::tuple {key}, std::forward<ValueFactoryFunc>(valueFactory)(key));
        }
        else
            m_cache.splice(m_cache.begin(), m_cache, iterator->second);

    	iterator->second = m_cache.begin();
    }


    [[nodiscard]] std::optional<const std::reference_wrapper<Value>> find(const key_type& key) const noexcept
    {
        if (auto it = m_index.find(key); it != m_index.end())
            return it->second->second;

        return std::nullopt;
    }

    [[nodiscard]] bool exists(const key_type& key) const noexcept { return m_index.contains(key); }
    [[nodiscard]] size_t capacity() const noexcept { return m_capacity; }

private:
    //TODO: arena allocator for list
	using ListType = std::list<value_type>;
    using ListIterator = typename ListType::iterator;
    using MapType = std::unordered_map<key_type, ListIterator>;

    ListType m_cache; 
	MapType m_index;
    size_t m_capacity;
};