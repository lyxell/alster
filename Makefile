CXXFLAGS=-Wall -Wfatal-errors -g
LDFLAGS=-lncurses

alster: alster.cpp buffer.cpp buffer.h
	g++ $(CXXFLAGS) alster.cpp buffer.cpp $(LDFLAGS) -o $@
