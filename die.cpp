#include "die.h"

Die::Die()
{
  m_Values.resize(6);
  for (int i = 0; i < 6; ++i)
    m_Values.at(i) = 'a' + (rand() % 26);
}

Die::Die(const std::string& values)
  : m_Values(values)
{
}

char Die::Roll()
{
  if (m_Values.empty())
    return 'a';
  return m_Values.at(rand() % m_Values.length());
}
