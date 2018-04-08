#pragma once

#ifndef __HashTable
#define __HashTable

#include <vector>
#include <cstddef>

struct Node
{
    void *Key;
	void *Value;

    struct Node *Next;
};
#ifdef LINUX
    #define __stdcall
#endif
//--------------------用于比较的函数指针
typedef bool (__stdcall *EQUALFUNC)(void *item, void *key);

//--------------------用于得到Hash代码的函数指针
typedef long (__stdcall *GETHASHFUNC)(void *key);


class HashTable
{
    public:
        HashTable(void);
        HashTable(long size);
        HashTable(long size, EQUALFUNC equalFunc, GETHASHFUNC getHashFunc);

    public:
        ~HashTable(void);

    // 公共方法

        void Add(void *key, void *value);         ///< 在哈希表里添加一个结点(入口点).key不能为NULL.

        void Remove(void *key);                     ///< 在哈希表里移除一个指定键的结点(入口点)

        void Clear();                                       ///< 清除所有结点.

        bool ContainsKey(void *key);              ///< 是否包含指定的键.

        Node* Find(void *key);                        ///< 获取指定键对应的结点.请不要修改Key和Next.

        long Get_Count();                                  ///< 得到哈希表中结点数目.

		long Get_FreeID();

		long Get_TableLength();

		void Resize(long newSize);                         ///< 重新分配大小

		void ClearFreePopID();

		void ClearFreeList();

		Node* GetFreeList();

    protected:
		long GetFreeListCount();

        long GetHash(void *key);         ///< 返回指定键的哈希代码.key不能为NULL.

        bool KeyEquals(void *item, void *key); ///< 将item和表中的key进行比较.

        EQUALFUNC EqualFunc;                      ///< 比较函数的指针.如果为NULL,则用DefaultEqualFunc.

        GETHASHFUNC GetHashFunc;             ///< 得到哈希代码的函数的指针.如果为NULL,则用DefaultGetHashFunc.

    private:

		long nextFreeID;

        bool inited;                                        ///< 是否初始化.

        long tableLength;                                  ///< 哈希表的长度

        Node **table;                                     ///< 存储键/值对的表.

        long nodeCount;                                   ///< 结点的数目.

        void init(long size, EQUALFUNC equalFunc, GETHASHFUNC getHashFunc); ///< 初始化.用于构造函数.

        Node *freeList;                                 ///< 保存删除的结点.当添加新节点时,先从这里取出结点,而不重新分配.
        void pushFreeList(Node *p);                    ///< 添加结点到freeList.
        Node *popFreeList();                           ///< 从freeList中弹出一个结点,为空则返回NULL.
		long freeListCount;

		std::vector <long> freePopID;					///将废弃的ID号存储，需要时首先从这里取出ID
		long popFreeID();								///弹出一个ID
		void pushFreeID( void* key );					///存储一个废弃ID
	public:
		long maxFreeListCount;
};
#endif
