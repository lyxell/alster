CXXFLAGS=-Wall -Wfatal-errors -O2 -g
LDFLAGS=-lncurses

alster: alster.cpp buffer.cpp tokenize/tokenize_c.cpp buffer.h
	$(CXX) $(CXXFLAGS) alster.cpp buffer.cpp tokenize/tokenize_c.cpp \
		$(LDFLAGS) -o $@
fuzzer: alster.cpp buffer.cpp tokenize/tokenize_c.cpp buffer.h
	clang++ -DFUZZ -g -O1 -fsanitize=fuzzer,address alster.cpp buffer.cpp tokenize/tokenize_c.cpp $(LDFLAGS) -o $@

%.cpp: %_re2c.cpp
	re2c -i $< -o $@
