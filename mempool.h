#pragma once
#include <new>
#include <string.h>
#include <iostream>

/// Linear memory allocator with no memory reuse
template<class T>
class Mempool
{
public:
  typedef T* value_type;

  Mempool(unsigned int initialSize)
    : m_PoolSize(initialSize)
    , m_PoolIndex(0)
    , m_CurrentPos(0)
  {
    m_Pool = (void**)malloc(sizeof(void*));
    m_Pool[0] = malloc(sizeof(T) * initialSize);
  }
  ~Mempool()
  {
    for (unsigned int index = 0; index <= m_PoolIndex; ++index)
    {
      free(m_Pool[index]);
    }
    free((void**)m_Pool);
  }

  T* Allocate()
  {
    if (m_CurrentPos >= m_PoolSize)
    {
      ++m_PoolIndex;
      //new pool index size
      void** newPool = (void**)malloc(sizeof(void*) * (m_PoolIndex + 1));
      //copy the old index
      memcpy(newPool, m_Pool, sizeof(void*) * m_PoolIndex);
      //allocate the new pool
      newPool[m_PoolIndex] = malloc(sizeof(T) * m_PoolSize);
      free(m_Pool);
      m_CurrentPos = 0;
      m_Pool = newPool;
    }

    T* pool = reinterpret_cast<T*>(m_Pool[m_PoolIndex]);
    pool += m_CurrentPos++;
    return new(pool)T();
  }

  T* Allocate(size_t count)
  {
    //store the char string, 0 terminator, left and right pointers
    size_t newPos = m_CurrentPos + (count * sizeof(T)) + 1 + (2 * sizeof(const char*));
    if (newPos >= m_PoolSize)
    {
      ++m_PoolIndex;
      //new pool index size
      void** newPool = (void**)malloc(sizeof(void*) * (m_PoolIndex + 1));
      //copy the old index
      memcpy(newPool, m_Pool, sizeof(void*) * m_PoolIndex);
      //allocate the new pool
      newPool[m_PoolIndex] = malloc(sizeof(T) * m_PoolSize);
      free(m_Pool);
      m_CurrentPos = 0;
      m_Pool = newPool;
    }
    T* pool = reinterpret_cast<T*>(m_Pool[m_PoolIndex]);
    pool += m_CurrentPos;
    m_CurrentPos += (count * sizeof(T)) + 1 + (2 * sizeof(const char*));
    return new(pool)T();
  }

  void PrintStats() const
  {
    unsigned int poolCount = m_PoolIndex + 1;
    unsigned int entries = m_PoolIndex * m_PoolSize + m_CurrentPos;
    std::cout << "We have a " << poolCount << " pools for a total  of " << sizeof(T) * m_PoolSize * poolCount << " bytes, " << entries << " entries for " << sizeof(T) * entries << " bytes used\n";
  }

private:
  unsigned int m_PoolSize;
  unsigned int m_PoolIndex;
  unsigned int m_CurrentPos;
  void** m_Pool;
};