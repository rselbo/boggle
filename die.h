#pragma once

#include <string>

class Die
{
public:
  //creates a random 6 sided die
  Die();
  //creates a values.length() sided die 
  Die(const std::string& values);

  char Roll();
//private:
  std::string m_Values;
};
