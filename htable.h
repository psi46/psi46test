// htable2.h

#pragma once

template <class T>
class CHashTable
{
	class CHashTableEntry
	{
		CHashTableEntry *next;
		CHashTableEntry *prev;
		char *s;
		T value;
	public:
		friend class CHashTable<T>;
	};
	unsigned int it;
	CHashTableEntry *itEl;

	unsigned int nEntries;
	unsigned int table_size;
	CHashTableEntry **table;
	unsigned int Hash(const char *key);
	bool FindEntry(const char *key, unsigned int &i, CHashTableEntry* &p);
public:
	CHashTable(unsigned int tablesize = 64);
	~CHashTable();
	unsigned int GetSize() { return nEntries; }
	T* Find(const char *key);
	bool Add(const char *key, const T data);
	bool Delete(const char *key);

	T* GetFirst()
	{
		for (it = 0; it < table_size; it++) if ( (itEl = table[it]) ) return &(itEl->value);
		return 0;
	}
	T* GetNext()
	{
		if (!itEl) return 0;
		if ( (itEl = itEl->next) ) return &(itEl->value);
		while (++it < table_size) if ( (itEl = table[it]) ) return &(itEl->value);
		return 0;
	}
	const char* GetKey() { return itEl ? itEl->s : 0; }
};


template <class T>
CHashTable<T>::CHashTable(unsigned int tablesize) : itEl(0), nEntries(0), table_size(tablesize)
{
	table = new CHashTableEntry*[table_size];
	for (unsigned int i=0; i<table_size; i++) table[i] = 0;
}


template <class T>
CHashTable<T>::~CHashTable()
{
	for (unsigned int i=0; i<table_size; i++)
	{
		while (table[i])
		{
			CHashTableEntry *p = table[i];
			table[i] = p->next;
			delete[] p->s;
			delete p;
		}
	}
}


template <class T>
unsigned int CHashTable<T>::Hash(const char *key)
{
   unsigned int hash = 5381;
   unsigned int i = 0;
   while (key[i])
   {
      hash = 33 * hash + key[i];
	  i++;
   }
   return hash % table_size;
}


template <class T>
bool CHashTable<T>::FindEntry(const char *key, unsigned int &i, CHashTableEntry* &p)
{
	i = Hash(key);
	CHashTableEntry *q = table[i];
	while (q)
	{
		if (strcmp(key, q->s) == 0) { p = q; return true; }
		q = q->next;
	}
	return false;
}


template <class T>
T* CHashTable<T>::Find(const char *key)
{
	unsigned int i;
	CHashTableEntry* p;
	if (FindEntry(key, i, p)) return &(p->value);
	return 0;
}


template <class T>
bool  CHashTable<T>::Add(const char *key, const T data)
{
	unsigned int i;
	CHashTableEntry* p;
	if (FindEntry(key, i, p)) return false;

	unsigned int size = strlen(key);
	p = new CHashTableEntry;
	p->s = new char[size+1];
	strcpy(p->s, key);
	p->value = data;
	
	p->prev = 0;
	p->next = table[i];
	if (p->next) p->next->prev = p;
	table[i] = p;

	nEntries++;
	return true;
}


template <class T>
bool CHashTable<T>::Delete(const char *key)
{
	unsigned int i;
	CHashTableEntry* p;
	if (!FindEntry(key, i, p)) return false;

	if (p->prev) p->prev->next = p->next; else table[i] = p->next;
	if (p->next) p->next->prev = p->prev;

	table[i] = p;
	delete p;
	nEntries--;
	return true;
}
