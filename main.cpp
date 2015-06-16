#include <map>
#include <time.h>
#include <vector>
#include <string.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "boggle.h"
#include "timer.h"

//dirty commandline parser
std::map<std::string, std::string> ParseCommandline(int argc, char** argv)
{
  static std::vector<std::string> optionNames;
  if (optionNames.empty())
  {
    optionNames.push_back("--help");
    optionNames.push_back("--dict");
    optionNames.push_back("--dice");
    optionNames.push_back("--width");
    optionNames.push_back("--height");
    optionNames.push_back("--pref");
  }

  std::map<std::string, std::string> options;
  options["--help"] = "";
  options["--dict"] = "data/sowpods.txt";
  options["--dice"] = "data/dice.txt";
  options["--width"] = "3";
  options["--height"] = "3";
  options["--pref"] = "speed";

  for (int i = 1; i < argc; ++i)
  {
    std::string cmd(argv[i]);
    if (std::find(optionNames.cbegin(), optionNames.cend(), cmd) != optionNames.cend())
    {
      if (cmd == "--help")
      {
        options[cmd] = "1";
      }
      else if (i+1 < argc)
      {
        options[cmd] = argv[++i];
      }
    }
  }

  return options;
}

int main(int argc, char** argv)
{
  srand(static_cast<unsigned int>(time(0)));
  /// Handle input parameters
  auto params = ParseCommandline(argc, argv);
  if (!params["--help"].empty())
  {
    std::cout << "Usage " << argv[0] << " [--help] [--dict <filename>] [--dice <filename>] [--width <number>] [--height <number>]" << std::endl;
    std::cout << "    --help              Prints this message" << std::endl;
    std::cout << "    --dict <filename>   Specify what dictionary file to use. (Default sowpods.txt)" << std::endl;
    std::cout << "    --dice <filename>   Specify what dice file to use. Specify random to create random dice. (Default dice.txt)" << std::endl;
    std::cout << "    --height <number>   Specify the height of the board. (Default 3)" << std::endl;
    std::cout << "    --width <number>    Specify the width of the board. (Default 3)" << std::endl;
    return 0;
  }
  std::string dictFileName(params["--dict"]);
  std::string diceFileName(params["--dice"]);
  int height = std::max(atoi(params["--height"].c_str()), 3);
  int width = std::max(atoi(params["--width"].c_str()), 3);

  std::string line;
  Boggle boggle(height, width);

  /// Dictionary load --------------------------------------------------------------------------------
  std::cout << "Starting dictionary load" << std::endl;
  Timer timer;
  timer.Start();
  std::fstream dictFile(dictFileName, std::ifstream::in);
  if (!dictFile.is_open())
  {
    std::cout << "Could not open dictionary file \"" << dictFileName << "\"" << std::endl;
    return -1;
  }
  //Fill the internal boggle dictionary
  while (std::getline(dictFile, line))
  { 
    boggle.AddWord(line);
  }
  dictFile.close();
  
  std::cout << "Finished dictionary load. Time elapsed " << static_cast<float>(timer.Stop()) / timer.GetResolution() << " seconds" << std::endl;


  /// Dice load --------------------------------------------------------------------------------
  std::cout << "Starting dice load" << std::endl;
  timer.Start();
  if (diceFileName == "random")
  {
    int boardlen = height * width;
    for (int i = 0; i < boardlen; ++i)
      boggle.AddDie();
  }
  else
  {
    std::fstream diceFile(diceFileName, std::ifstream::in);
    if (!diceFile.is_open())
    {
      std::cout << "Could not open dice file \"" << dictFileName << "\"" << std::endl;
      return -1;
    }
    //Fill the internal boggle dictionary
    while (std::getline(diceFile, line))
    {
      //trim leading and trailing whitespaces
      line = line.substr(line.find_first_not_of(" \r\n"), line.find_last_of(" \r\n"));
      boggle.AddDie(line);
    }
    diceFile.close();
  }
  std::cout << "Finished dice load. Time elapsed " << static_cast<float>(timer.Stop()) / timer.GetResolution() << " seconds" << std::endl;

  
  std::cout << "Starting solve" << std::endl;
  timer.Start();;
  boggle.CreateBoard();
  boggle.Solve();
  std::cout << "Finished solve. Time elapsed " << static_cast<float>(timer.Stop()) / timer.GetResolution() << " seconds" << std::endl;

  boggle.PrintStats();

  return 0;
}