CXX=g++
CXXFLAGS = -std=c++11 -Wall -Wno-pointer-arith -g -DPOSIX
LFLAGS = -lpthread
OUTPUT=boggle
CONFIGURATION = Release
FILES = boggle.cpp boggle.h die.cpp die.h main.cpp mempool.h
OBJS = $(CONFIGURATION)/die.o $(CONFIGURATION)/boggle.o $(CONFIGURATION)/main.o

$(CONFIGURATION)/%.o: %.cpp 
	@mkdir -p $(CONFIGURATION)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

boggle: $(OBJS)
	$(CXX) -o $(CONFIGURATION)/$(OUTPUT) $(CXXFLAGS) $(LFLAGS) $^

.PHONY: clean
clean:
	rm -rf $(CONFIGURATION)
