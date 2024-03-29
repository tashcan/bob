#pragma once

// Mono code also has define for GROUP_SIZE, so we need to wrap its usage here
#pragma push_macro("GROUP_SIZE")
#undef GROUP_SIZE
#if IL2CPP_USE_SPARSEHASH
#include "../external/google/sparsehash/sparse_hash_map.h"
#else
#include "../external/google/sparsehash/dense_hash_map.h"
#endif
#pragma pop_macro("GROUP_SIZE")

#include "KeyWrapper.h"

#include "../os/FastReaderReaderWriterLock.h"

template<class Key, class T,
         class HashFcn = SPARSEHASH_HASH<Key>,
         class EqualKey = std::equal_to<Key>,
         class Alloc = GOOGLE_NAMESPACE::libc_allocator_with_realloc<std::pair<const KeyWrapper<Key>, T> > >
#if IL2CPP_USE_SPARSEHASH
class Il2CppHashMap : public GOOGLE_NAMESPACE::sparse_hash_map<KeyWrapper<Key>, T, HashFcn, typename KeyWrapper<Key>::template EqualsComparer<EqualKey>, Alloc>
#else
class Il2CppHashMap : public GOOGLE_NAMESPACE::dense_hash_map<KeyWrapper<Key>, T, HashFcn, typename KeyWrapper<Key>::template EqualsComparer<EqualKey>, Alloc>
#endif
{
private:
#if IL2CPP_USE_SPARSEHASH
    typedef GOOGLE_NAMESPACE::sparse_hash_map<KeyWrapper<Key>, T, HashFcn, typename KeyWrapper<Key>::template EqualsComparer<EqualKey>, Alloc> Base;
#else
    typedef GOOGLE_NAMESPACE::dense_hash_map<KeyWrapper<Key>, T, HashFcn, typename KeyWrapper<Key>::template EqualsComparer<EqualKey>, Alloc> Base;
#endif

public:
    typedef typename Base::size_type size_type;
    typedef typename Base::hasher hasher;
    typedef typename Base::key_equal key_equal;
    typedef typename Base::key_type key_type;

    explicit Il2CppHashMap(size_type n = 0,
                           const hasher& hf = hasher(),
                           const EqualKey& eql = EqualKey()) :
        Base(n, hf, key_equal(eql))
    {
        Base::set_deleted_key(key_type(key_type::KeyType_Deleted));
#if !IL2CPP_USE_SPARSEHASH
        Base::set_empty_key(key_type(key_type::KeyType_Empty));
#endif
    }

    template<class InputIterator>
    Il2CppHashMap(InputIterator f, InputIterator l,
                  size_type n = 0,
                  const hasher& hf = hasher(),
                  const EqualKey& eql = EqualKey()) :
        Base(f, l, n, hf, key_equal(eql))
    {
        Base::set_deleted_key(key_type(key_type::KeyType_Deleted));
#if !IL2CPP_USE_SPARSEHASH
        Base::set_empty_key(key_type(key_type::KeyType_Empty));
#endif
    }

    void add(const key_type& key, const T& value)
    {
        Base::insert(std::make_pair(key, value));
    }
};

template<class Key, class T,
         class HashFcn = SPARSEHASH_HASH<Key>,
         class EqualKey = std::equal_to<Key>,
         class Alloc = GOOGLE_NAMESPACE::libc_allocator_with_realloc<std::pair<const KeyWrapper<Key>, T> > >
class Il2CppReaderWriterLockedHashMap
{
public:
    typedef typename Il2CppHashMap<Key, T, HashFcn, EqualKey, Alloc>::key_type key_type;
    typedef typename Il2CppHashMap<Key, T, HashFcn, EqualKey, Alloc>::size_type size_type;
    typedef typename Il2CppHashMap<Key, T, HashFcn, EqualKey, Alloc>::const_iterator const_iterator;
    typedef typename Il2CppHashMap<Key, T, HashFcn, EqualKey, Alloc>::iterator iterator;
    typedef typename Il2CppHashMap<Key, T, HashFcn, EqualKey, Alloc>::hasher hasher;

    explicit Il2CppReaderWriterLockedHashMap(size_type n = 0,
                                             const hasher& hf = hasher(),
                                             const EqualKey& eql = EqualKey()) :
        hashMap(n, hf, eql)
    {
    }

    bool TryGet(const key_type& key, T* value)
    {
        il2cpp::os::FastReaderReaderWriterAutoSharedLock readerLock(&lock);
        const_iterator iter = hashMap.find(key);
        if (iter != hashMap.end())
        {
            *value = iter->second;
            return true;
        }
        return false;
    }

    bool Add(const key_type& key, const T& value)
    {
        il2cpp::os::FastReaderReaderWriterAutoExclusiveLock writerLock(&lock);
        return hashMap.insert(std::make_pair(key, value)).second;
    }

    void Clear()
    {
        il2cpp::os::FastReaderReaderWriterAutoExclusiveLock writerLock(&lock);
        hashMap.clear();
    }

    void Remove(const key_type& key)
    {
        il2cpp::os::FastReaderReaderWriterAutoExclusiveLock readerLock(&lock);
        hashMap.erase(key);
    }

    // This function takes no locks, some other lock must be used to protect accesses
    iterator UnlockedBegin()
    {
        return hashMap.begin();
    }

    // This function takes no locks, some other lock must be used to protect accesses
    iterator UnlockedEnd()
    {
        return hashMap.end();
    }

private:
    il2cpp::os::FastReaderReaderWriterLock lock;
    Il2CppHashMap<Key, T, HashFcn, EqualKey, Alloc> hashMap;
};
