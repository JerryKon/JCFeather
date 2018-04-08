#include "HashTable.h"

long  __stdcall defalutGetHash(void *key)
{
        return *((long*)key);
};

//----------------------------------------------------------
bool  __stdcall defaultEqual(void *key, void *item)
{
	long tempItem = *((long*)item);
	long tempKey  = *((long*)key);
	return ( tempItem == tempKey ) ? true : false;
};

//----------------------------------------------------------
HashTable::HashTable(void)
{
	init(1,NULL,NULL);
}

//----------------------------------------------------------
HashTable::HashTable(long size)
{
    init(size, NULL, NULL);
}

//----------------------------------------------------------
HashTable::HashTable(long size, EQUALFUNC equalFunc, GETHASHFUNC getHashFunc)
{
    init(size, equalFunc, getHashFunc);
}

//----------------------------------------------------------
void HashTable::init(long size, EQUALFUNC equalFunc, GETHASHFUNC getHashFunc)
{

	this->inited = false;

	//----分配哈希表
    this->table = new Node*[size];
    if(this->table== NULL)
         return;
    for(int i=0; i<size; i++)
    {
         this->table[i]=NULL;
    }

    //----初始化其它值
    this->tableLength = size;
    this->nodeCount = 0;
    this->freeList = NULL;
	this->freeListCount = 0;
	this->maxFreeListCount = size;
	this->nextFreeID = 0;
	this->freePopID.clear();

    if(equalFunc != NULL)
         this->EqualFunc = equalFunc;
    else
         this->EqualFunc = defaultEqual;

    if(getHashFunc != NULL)
         this->GetHashFunc = getHashFunc;
    else
         this->GetHashFunc = defalutGetHash;



    this->inited =true;
}
//----------------------------------------------------------
HashTable::~HashTable(void)
{
     Clear();
     delete[] this->table;
}
//----------------------------------------------------------
void HashTable::Add(void *key, void *value)
{
     if(key==NULL)
     {
         return;
     }
     Node *pe = popFreeList();
     if(pe == NULL)
     {
         pe = new Node;
     }
     pe->Key = key;
     pe->Value = value;

     long k = GetHash(key) % tableLength;
     pe->Next = table[k];
     table[k] = pe;

     nodeCount++;

     if(nodeCount > tableLength)
     {
         Resize( 2*tableLength );
     }
}
//----------------------------------------------------------
void HashTable::Remove(void *key)
{
     long k = GetHash(key) % tableLength;
     Node *p = table[k], *q = NULL;
     while(p != NULL)
     {
         if(KeyEquals(key, p->Key))
         {
             if(q == NULL)
             {
                 table[k] = p->Next;
             }
			 else
			 {
				 q->Next = p->Next;
			 }
			 pushFreeList(p);
			 pushFreeID( key );
			 nodeCount--;
			 break;
         }
         q = p;
         p = p->Next;
      }
}

//----------------------------------------------------------
void HashTable::Clear()
{
     Node *p, *q;
     for(int i = 0; i< tableLength; i++)
     {
         p = table[i];
         while(p != NULL)
		 {
			 q = p->Next;
             pushFreeList(p);
             p = q;
         }
         table[i] = NULL;
     }
     nodeCount = 0;
	 nextFreeID = 0;
}

//----------------------------------------------------------
bool HashTable::ContainsKey(void *key)
{
     return ( Find(key) != NULL );
}

//----------------------------------------------------------
Node* HashTable::Find(void *key)
{
     long k = GetHash(key) % tableLength;
     Node *p = table[k];
     while(p != NULL)
     {
          if(KeyEquals(key, p->Key))
          {
                return p;
          }
		  p = p->Next;
     }
     return NULL;
}

//----------------------------------------------------------
long HashTable::Get_Count()
{
	return this->nodeCount;
}

//----------------------------------------------------------
long HashTable::Get_FreeID()
{
	long  newID = popFreeID();
	if( newID != -1 )
		return newID;
	else
		return this->nextFreeID++;
}

//----------------------------------------------------------
long HashTable::Get_TableLength()
{
	return this->tableLength;
}
//----------------------------------------------------------
long HashTable::GetHash(void *key)
{
     long i = GetHashFunc(key);
     return i;
}

//----------------------------------------------------------
bool HashTable::KeyEquals(void *item, void *key)
{
     return EqualFunc(item, key);
}

//----------------------------------------------------------
void HashTable::Resize( long newSize )
{
    if(!inited)
         return;
	Node *head = NULL;
    Node *p, *q;

    //保存所有Node到head
    for(int i = 0; i< tableLength; i++)
    {
         p = table[i];
         while(p != NULL)
         {
             q = p->Next;
             p->Next = head;
             head = p;
             p = q;
         }
    }

    //释放旧哈希表
    delete[] table;

    //得到新的哈希表长度
    tableLength = newSize;

    table = new Node*[tableLength];
    if(table== NULL)
    {
         inited = false;
         return;
    }
    for(int i=0; i<tableLength; i++)
    {
         table[i]=NULL;
    }

    //重新将Node添加到表中
    while(head != NULL)
    {
         p = head->Next;
         long k = GetHash(head->Key) % tableLength;
         head->Next = table[k];
         table[k] = head;
         head = p;
    }
}

//----------------------------------------------------------
void HashTable::pushFreeList(Node *p)
{
    p->Next = freeList;
    freeList = p;
	this->freeListCount++;

	ClearFreeList();
}

//----------------------------------------------------------
Node* HashTable::popFreeList()
{
    Node *p=freeList;
    if(freeList != NULL)
    {
        freeList = freeList->Next;
		this->freeListCount--;
    }
    return p;
}

//----------------------------------------------------------
long HashTable::popFreeID()
{
	if( freePopID.size() == 0 )
		return -1;
	else
	{
		long popedFreeID = freePopID.back();
		freePopID.pop_back();
		return popedFreeID;
	}
}

//----------------------------------------------------------
void HashTable::pushFreeID( void *key )
{
	freePopID.push_back( GetHash(key) );
}

void HashTable::ClearFreePopID()
{
	freePopID.clear();
}

void HashTable::ClearFreeList()
{
	if(GetFreeListCount() <= maxFreeListCount) return;

	Node *p = popFreeList();
	while(p)
	{
		delete p;
		p = popFreeList();
	}
	freeList = NULL;
}

Node* HashTable::GetFreeList()
{
	return this->freeList;
}

long HashTable::GetFreeListCount()
{
	return this->freeListCount;
}
