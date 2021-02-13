CXXFLAGS=-Wall -Wfatal-errors -g
LDFLAGS=-lncurses

alster: alster.cpp buffer.cpp tokenize/tokenize_c.cpp buffer.h
	g++ $(CXXFLAGS) alster.cpp buffer.cpp tokenize/tokenize_c.cpp \
		$(LDFLAGS) -o $@

%.cpp: %_re2c.cpp
	re2c -i $< -o $@
