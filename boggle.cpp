#include <sstream>
#include <iostream>
#include <algorithm>

#include "boggle.h"

Boggle::Boggle(int height, int width)
  : m_Pool(10000)
  , m_Base(nullptr)
  , m_Board(nullptr)
  , m_BoardHeight(height)
  , m_BoardWidth(width)
  , m_Score(0)
  , m_DictWords(0)
  , m_SolvedPositions(0)
{
  m_Base = m_Pool.Allocate();
#if defined(POSIX)
  pthread_mutex_init(&m_ThreadMutex, NULL);
#else
  m_ThreadMutex = CreateMutex(nullptr, false, nullptr);
#endif
}

Boggle::~Boggle()
{
  delete[] m_Board;
  for (auto i : m_Dice)
    delete i;

#if defined(POSIX)
  pthread_mutex_destroy(&m_ThreadMutex);
#else
  CloseHandle(m_ThreadMutex);
#endif
}

/// Adds a single word to the internal dictionary
void Boggle::AddWord(const std::string& line)
{
  AddLetter(line, 0, m_Base);
  ++m_DictWords;
}

/// Adds a single letter of a word to the internal dictionary
void Boggle::AddLetter(const std::string& word, unsigned int pos, DictEntry* dictionary)
{
#if defined(OPTIMIZED)
  DictEntry *entry = dictionary->Get(word[pos]);
  if (entry == nullptr)
  {
    entry = m_Pool.Allocate();
    entry->m_Character = word[pos];
    dictionary->Insert(entry);
  }
#else
  DictEntry *&entry = dictionary->m_NextLetter[word[pos] - 'a'];
  //Make sure we have an entry for this character
  if (entry == nullptr)
  {
    //Allocate and initialize the entry
    entry = m_Pool.Allocate();
    entry->m_Character = word[pos];
  }
#endif

  if (pos == word.length() - 1)
  {
    //if the word ends at this DictEntry mark the word as terminated at this entry
    entry->m_Terminated = true;
    return;
  }

  //recurse to add the next letter of the word
  AddLetter(word, ++pos, entry);
}


void Boggle::AddDie(const std::string& line)
{
  m_Dice.push_back(new Die(line));
}

void Boggle::AddDie()
{
  m_Dice.push_back(new Die);
}

void Boggle::CreateBoard()
{
  if (m_Dice.empty())
  {
    m_Dice.push_back(new Die);
  }
  auto diceCopy = m_Dice;
  unsigned int boardLen = m_BoardHeight * m_BoardWidth;
  m_Board = new char[boardLen];
  for (unsigned int i = 0; i < boardLen; ++i)
  {
    if (diceCopy.empty())
    {
      diceCopy = m_Dice;
    }
    int dieIndex = rand() % diceCopy.size();
    char c = diceCopy[dieIndex]->Roll();
    diceCopy.erase(diceCopy.begin() + dieIndex);
    m_Board[i] = c;
  }

// reference board, comment in for testing
   //static char staticBoard[] = { 'z', 'p', 'w', 'e', 'a', 'u', 'f', 's', 'x' };
   //m_Board = new char[sizeof(staticBoard)];
   //memcpy(m_Board, staticBoard, sizeof(staticBoard));
   //m_BoardHeight = m_BoardWidth = 3;
}

#if defined(POSIX)
void* Boggle::SolveThread(void* arg)
#else
DWORD WINAPI Boggle::SolveThread(void* arg)
#endif
{
  Boggle* boggle = reinterpret_cast<Boggle*>(arg);

  //set up a used letters bool array
  unsigned int boardLen = boggle->m_BoardHeight * boggle->m_BoardWidth;
  unsigned int* usedLetters = new unsigned int[(boardLen / (sizeof(unsigned int) * 8)) + 1];

  char* wordSoFar = new char[boardLen + 1];
  unsigned int boardPos = 0;
  while (true)
  {
    boardPos = boggle->m_SolvedPositions.fetch_add(1);
    if (boardPos >= boardLen)
      break;

    //make sure the usedLetters is blank
    memset(usedLetters, 0, sizeof(unsigned int) * ((boardLen / 32) + 1));
    boggle->SolveLetter(boardPos, boggle->m_Base, usedLetters, wordSoFar, 0);
  }
  delete[] usedLetters;
  delete[] wordSoFar;

  return 0;
}

//Solves the board and calculates the score
void Boggle::Solve()
{
  unsigned int boardLen = m_BoardHeight * m_BoardWidth;
  static const int threadCount = 25;
#if defined(POSIX)
  pthread_t threadInfo[threadCount];
  for(unsigned int solved = 0; solved < threadCount && solved < boardLen ; ++solved)
  {
    pthread_create(&threadInfo[solved], nullptr, &Boggle::SolveThread, this);
  }

  //wait for all threads to be finished
  for (unsigned int i = 0; i < threadCount && i < boardLen; ++i)
  {
    pthread_join(threadInfo[i], nullptr);
  }
#else
  HANDLE threadInfo[threadCount];
  for (unsigned int solved = 0; solved < threadCount && solved < boardLen; ++solved)
  {
    threadInfo[solved] = CreateThread(nullptr, 0, Boggle::SolveThread, this, 0, nullptr);
  }

  WaitForMultipleObjects(threadCount, threadInfo, true, INFINITE);
#endif

  //////set up a used letters bool array
  //unsigned int* usedLetters = new unsigned int[(boardLen / (sizeof(unsigned int) * 8)) + 1];

  //char* wordSoFar = new char[boardLen + 1];
  ////start solving from each character on the board
  //for (unsigned int pos = 0; pos < boardLen; ++pos)
  //{
  //  //make sure the usedLetters is blank
  //  memset(usedLetters, 0, sizeof(unsigned int) * ((boardLen / 32) + 1));
  //  SolveLetter(pos, m_Base, usedLetters, wordSoFar, 0);
  //}

  //delete[] usedLetters;
  //delete[] wordSoFar;
}

bool Boggle::SolveLetter(unsigned int pos, const DictEntry* entry, unsigned int* usedLetters, char* wordSoFar, unsigned int iterations)
{
  wordSoFar[iterations] = m_Board[pos]; wordSoFar[iterations + 1] = 0;

  int boardLen = m_BoardHeight * m_BoardWidth;

  usedLetters[(pos / (sizeof(unsigned int) * 8))] |= 1 << (pos % (sizeof(unsigned int) * 8));

  //check if this letter is valid for this dictionary entry, if not this path is failed
#if defined(OPTIMIZED)
  DictEntry* next = entry->Get(m_Board[pos]);
  if (next)
  {
    //if the word is terminated at this entry we have found a word of at least 3 characters
    if (next->m_Terminated)
#else
  unsigned int boardPos = m_Board[pos] - 'a';
  if (entry->m_NextLetter[boardPos])
  {
    //if the word is terminated at this entry we have found a word of at least 3 characters
    if ((entry->m_NextLetter[boardPos]->m_Terminated))
#endif
    {
      AddSolvedWord(wordSoFar);
    }

    int line = pos / m_BoardWidth;
    int col = pos % m_BoardWidth;
    //test the next letter
    for (int i = -1; i <= 1; ++i)
    {
      int newLine = line + i;
      for (int j = -1; newLine >= 0 && j <= 1; ++j)
      {
        int newCol = col + j;
        int index = newLine * m_BoardWidth + newCol;
        if (newLine >= 0 && newLine < static_cast<int>(m_BoardHeight) && newCol >= 0 && newCol < static_cast<int>(m_BoardWidth) && index < boardLen && index != static_cast<int>(pos))
        {
          if (!(usedLetters[(index / (sizeof(unsigned int) * 8))] & 1 << (index % (sizeof(unsigned int) * 8))))
          {
#if defined(OPTIMIZED)
            SolveLetter(index, next, usedLetters, wordSoFar, iterations + 1);
#else
            SolveLetter(index, entry->m_NextLetter[boardPos], usedLetters, wordSoFar, iterations + 1);
#endif
            wordSoFar[iterations + 1] = 0;
            //clear the last used letter since we have tested that position 
            usedLetters[(index / (sizeof(unsigned int) * 8))] &= ~(1 << (index % (sizeof(unsigned int) * 8)));
          }
        }
      }
    }
  }
  return false;
}

void Boggle::AddSolvedWord(const std::string& word)
{

#if defined(POSIX)
  pthread_mutex_lock(&m_ThreadMutex);
#else
  WaitForSingleObject(m_ThreadMutex, INFINITE);
#endif
  //check if we already know this word
  if (m_SolvedWords.insert(word).second)
  {
    if (word.length() == 3 || word.length() == 4) m_Score += 1;
    else if (word.length() == 5) m_Score += 2;
    else if (word.length() == 6) m_Score += 3;
    else if (word.length() == 7) m_Score += 5;
    else if (word.length() >= 8) m_Score += 11;
  }
#if defined(POSIX)
  pthread_mutex_unlock(&m_ThreadMutex);
#else
  ReleaseMutex(m_ThreadMutex);
#endif
}

void Boggle::PrintStats() const
{
  std::cout << std::endl;
  for (unsigned int i = 0; i < m_BoardHeight; ++i)
  {
    std::cout << "[ ";
    for (unsigned int j = 0; j < m_BoardWidth; ++j)
    {
      std::cout << m_Board[i*m_BoardWidth + j] << " ";
    }
    std::cout << "]" << std::endl;
  }

  std::cout << std::endl << "Score: " << m_Score << " points" << std::endl;
  std::cout << m_SolvedWords.size() << " words: ";
  bool first = true;
  for (const std::string& word : m_SolvedWords)
  {
    if (first)
    {
      std::cout << word;
      first = false;
    }
    else
    {
      std::cout << ", " << word;
    }
  }
  std::cout << std::endl;
  std::cout << "We have " << m_DictWords << " words in the dictionary" << std::endl;
  m_Pool.PrintStats();

}
