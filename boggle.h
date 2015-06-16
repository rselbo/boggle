#pragma once
#include <set>
#include <vector>
#include <cassert>
#include <string.h>
#include <atomic>
#if defined(POSIX)
#include <pthread.h>
#else
#define NOMINMAX
#include <windows.h>
#endif

#include "mempool.h"
#include "die.h"

//Memory optimization at the cost of complexity
#define OPTIMIZED

struct DictEntry
{
#if defined(OPTIMIZED)
  DictEntry() : m_Character('\0'), m_Terminated(false), m_NextLetter(nullptr), m_Siblings(nullptr){}
  //caller is responsible for making sure there are no duplicate letters inserted 
  void Insert(DictEntry* entry)
  {
    if (m_NextLetter == nullptr)
    {
      m_NextLetter = entry;
    }
    else
    {
      DictEntry* siblings = m_NextLetter;
      DictEntry* lastSibling = nullptr;
      while (siblings != nullptr)
      {
        assert(m_NextLetter->m_Character != entry->m_Character);
        //is this sibling in front the new entry in the list
        if (siblings->m_Character < entry->m_Character)
        {
          //do we have any siblings
          if (siblings->m_Siblings == nullptr)
          {
            //we have no siblings so add this to the sibling list
            siblings->m_Siblings = entry;
            return;
          }
          //move to the next sibling
          lastSibling = siblings;
          siblings = siblings->m_Siblings;
        }
        else //we found a spot for this entry
        {
          //are we taking the first siblings spot
          if (siblings == m_NextLetter)
          {
            m_NextLetter = entry;
            m_NextLetter->m_Siblings = siblings;
          }
          else
          {
            //we are not the first sibling so link us into this sibling
            lastSibling->m_Siblings = entry;
            entry->m_Siblings = siblings;
          }
          return;
        }
      }
    }
  }
  DictEntry* Get(char c) const
  {
    //do we have any children, if not return nullptr
    if (m_NextLetter == nullptr) return nullptr;

    //start searching
    DictEntry* nextLetter = m_NextLetter;
    while (nextLetter != nullptr)
    {
      if (nextLetter->m_Character == c) return nextLetter;
      if (nextLetter->m_Character > c) return nullptr;
      nextLetter = nextLetter->m_Siblings;
    }
    return nullptr;
  }
  char m_Character;
  bool m_Terminated;
  //Points to the lowest character
  DictEntry* m_NextLetter;
  //This forms a single linked list of siblings on this level
  DictEntry* m_Siblings;

#else
  DictEntry() : m_Character('\0'), m_Terminated(false) { memset(m_NextLetter, 0, sizeof(m_NextLetter)); }
  char m_Character;
  bool m_Terminated;
  DictEntry* m_NextLetter[26];

#endif

};

class Boggle
{
public:
  Boggle(int height, int width);
  ~Boggle();
  void AddWord(const std::string& line);
  void AddDie(const std::string& line);
  void AddDie();

  void CreateBoard();
  void Solve();

  void PrintStats() const;
protected:
  void AddSolvedWord(const std::string& word);
  void AddLetter(const std::string& word, unsigned int pos, DictEntry* entry);
  bool SolveLetter(unsigned int pos, const DictEntry* entry, unsigned int* usedLetters, char* wordSoFar, unsigned int iterations);
#if defined(POSIX)
  static void* SolveThread(void* arg);
#else
  static DWORD WINAPI SolveThread(void* arg);
#endif

  Mempool<DictEntry> m_Pool;
  DictEntry* m_Base;

  char* m_Board;
  unsigned int m_BoardHeight;
  unsigned int m_BoardWidth;
  unsigned int m_Score;
  std::set<std::string> m_SolvedWords;
  std::vector<Die*> m_Dice;
  unsigned int m_DictWords;
  //used by threads to know when to end
  std::atomic<unsigned int> m_SolvedPositions;
#if defined(POSIX)
  pthread_mutex_t m_ThreadMutex;
#else
  HANDLE m_ThreadMutex;
#endif
};
