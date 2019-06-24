#pragma once

#include <Core/Misc/Defines/Common.hpp>
#include <Core/Misc/Defines/Debug.hpp>
#include <Core/DataStructure/String/String.hpp>
#include <Core/Memory/Allocators/PoolAlloc/MultiPoolAllocator.hpp>

// Based on this : https://github.com/Bibeknam/algorithmtutorprograms/blob/master/data-structures/red-black-trees/RedBlackTree.cpp
// https://codereview.stackexchange.com/questions/153476/iterator-through-binary-trees-with-parent-pointer-without-memory-overhead

TRE_NS_START

template<typename T>
static bool Compare(const T& a, const T& b)
{
	return a < b;
}

template<typename K, typename T, typename Alloc_t = MultiPoolAlloc>
class RedBalckTree
{
public:
	class Iterator;

	struct RedBlackNode {
		K key;					// holds the key
		T value;				// holds the value
		RedBlackNode* parent;	// pointer to the parent
		RedBlackNode* left;		// pointer to left child
		RedBlackNode* right;	// pointer to right child
		bool color;				// true -> Red, false -> Black

		template<typename... Args>
		RedBlackNode(RedBlackNode* p, RedBlackNode* r, RedBlackNode* l, const K& key, Args&&... args) : 
			color(true), parent(p), right(r), left(l), key(key), value(std::forward<Args>(args)...)
		{}

		RedBlackNode* GetRight() { return right; }
		RedBlackNode* GetLeft() { return left; }
		RedBlackNode* GetParent() { return parent; }
		T& GetValue() { return value; }
		K& GetKey() { return key; }

		void SetRight(RedBlackNode* r) { right = r; }
		void SetLeft(RedBlackNode* l) { left = l; }
		void SetParent(RedBlackNode* p) { parent = p; }
	};

	typedef RedBlackNode RBNode;
	typedef RedBlackNode RBLeaf;

	FORCEINLINE RedBalckTree();

	virtual ~RedBalckTree();

	template<typename... Args>
	FORCEINLINE RedBalckTree(const K& key, Args&&... args);

	FORCEINLINE RBNode* Search(const K& key) const;

	template<typename... Args>
	FORCEINLINE T& Insert(const K& key, Args&&... args);

	FORCEINLINE void Remove(const K& key);

	FORCEINLINE void LeftRotate(RBNode* x);
	FORCEINLINE void RightRotate(RBNode* x);

	// find the node with the minimum key
	FORCEINLINE RBNode* Min(RBNode* node);

	// find the node with the maximum key
	FORCEINLINE RBNode* Max(RBNode* node);

	FORCEINLINE void Clear();

	FORCEINLINE bool IsEmpty() const;

	FORCEINLINE void Print();

	FORCEINLINE Iterator begin() noexcept;

	FORCEINLINE Iterator end() noexcept;

private:
	CONSTEXPR static const usize NODE_CHUNKS = 3;
	CONSTEXPR static RBNode* TNULL = reinterpret_cast<RBNode*>(-1);

	RBNode* m_Root;
	Alloc_t m_Allocator;

	FORCEINLINE void InsertHelper(RBNode* newNode, RBNode* parent);

	FORCEINLINE void DestroyTree(RBNode* node);

	// initializes the nodes with appropirate values
	// all the pointers are set to point to the null pointer
	FORCEINLINE void InitNullNode(RBNode* node, RBNode* parent);

	FORCEINLINE RBNode* SearchTreeHelper(RBNode* node, const K& key) const;

	FORCEINLINE void RBTransplant(RBNode* u, RBNode* v);

	// fix the rb tree modified by the delete operation
	void FixDelete(RBNode* x);

	void DeleteNodeHelper(RBNode* node, const K& key);

	// fix the red-black tree
	void FixInsert(RBNode* k);

	FORCEINLINE void PrintHelper(RBNode* node, String indent, bool last);

	class Iterator
	{
	public:
		Iterator(RedBalckTree<K, T, Alloc_t>* instance) : m_TreeInstance(instance), m_Node(instance->m_Root)
		{};

		Iterator(RedBalckTree<K, T, Alloc_t>* instance, RedBlackNode* node) : m_TreeInstance(instance), m_Node(node)
		{};
		
		bool operator!=(const Iterator& iterator) { return m_Node != iterator.m_Node; }

		RBNode& operator*() const { return *m_Node; }

		Iterator& operator=(const Iterator& other)
		{
			this->m_Node = other.m_Node;
			this->m_TreeInstance = other.m_TreeInstance;
			return *this;
		}

		Iterator& operator++()
		{
			RBNode* parent = NULL;

			if (this->m_Node == NULL || this->m_Node == TNULL) {
				return *this; // end iterator does not increment
			}

			parent = this->m_Node->parent;

			// reaches root -> next is end()
			if (parent == NULL || parent == TNULL) {
				this->m_Node = NULL;
				return *this;
			}

			// left child -> go to right child
			if ((this->m_Node == parent->left) && (parent->right != NULL || parent->right != TNULL)) {
				this->m_Node = parent->right;
			}else{
				this->m_Node = this->m_Node->parent;
				return *this;
			}

			while (true) {
				if (this->m_Node->left != NULL || this->m_Node->left != TNULL) {
					this->m_Node = this->m_Node->left; // has left child node
				}else if (this->m_Node->right != NULL || this->m_Node->right != TNULL) {
					this->m_Node = this->m_Node->right; // only right child node
				}else {
					return *this; // has no children -> stop here
				}
			}
		}

		Iterator operator++(int)
		{
			Iterator iterator = *this;
			++(*this);
			return iterator;
		}

		Iterator& operator--()
		{
			// TODO: operator -- have to be implemented
			return *this;
		}

		Iterator operator--(int)
		{
			Iterator iterator = *this;
			--*this;
			return iterator;
		}

	private:
		RedBlackNode* m_Node;
		RedBalckTree<K, T, Alloc_t>* m_TreeInstance;
	};
};

#include "RedBlackTree.inl"

//template<typename K, typename T, typename Alloc_t = MultiPoolAlloc>
//using RBTree = RedBalckTree<K, T, Alloc_t>;

template<typename K, typename T, typename Alloc_t = MultiPoolAlloc>
using RBT = RedBalckTree<K, T, Alloc_t>;

TRE_NS_END

