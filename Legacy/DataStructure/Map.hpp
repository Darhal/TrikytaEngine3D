#pragma once

#include <Legacy/Misc/Defines/Common.hpp>
#include <Legacy/Misc/Defines/Debug.hpp>
#include <Legacy/DataStructure/RedBlackTree.hpp>

TRE_NS_START

template<typename K, typename V, typename Alloc_t = MultiPoolAlloc>
class Map
{
public:
	typedef typename RBT<K, V, Alloc_t>::Iterator Iterator;
	typedef typename RBT<K, V, Alloc_t>::CIterator CIterator;
	typedef RBT<K, V, Alloc_t> MapTree;
	typedef typename MapTree::RedBlackNode Element;

	FORCEINLINE Map(const Alloc_t& alloc) : m_RBT(alloc) {}

	FORCEINLINE Map();

	FORCEINLINE Map(Map&& other);

	FORCEINLINE Map(const Map& other);

	FORCEINLINE Map& operator=(Map&& other);

	FORCEINLINE Map& operator=(const Map& other);

	FORCEINLINE V& Put(const K& key, const V& value);

	template<typename... Args>
	FORCEINLINE V& Emplace(const K& key, Args&&... args);

	FORCEINLINE V* Get(const K& key) const;

	FORCEINLINE V* At(const K& key) const;

	FORCEINLINE V& operator[](const K& key);

	FORCEINLINE const V& operator[](const K& key) const;

	FORCEINLINE void Remove(const K& key);

	FORCEINLINE bool ContainsKey(const K& key) const;

	FORCEINLINE void Clear();

	FORCEINLINE bool IsEmpty() const;

	FORCEINLINE Iterator begin() noexcept;

	FORCEINLINE Iterator end() noexcept;

	FORCEINLINE CIterator begin() const noexcept;

	FORCEINLINE CIterator end() const noexcept;

	FORCEINLINE CIterator cbegin() const noexcept;

	FORCEINLINE CIterator cend() const noexcept;

	FORCEINLINE Iterator rbegin() noexcept;

	FORCEINLINE Iterator rend() noexcept;

	FORCEINLINE CIterator crbegin() const noexcept;

	FORCEINLINE CIterator crend() const noexcept;
private:
	RBT<K, V, Alloc_t> m_RBT;
};

#include "Map.inl"

TRE_NS_END

