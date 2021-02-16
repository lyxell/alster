CXXFLAGS=-std=c++17 -Wall -Wfatal-errors -O2 -g
LDFLAGS=-lncurses

alster: alster_re2c.cpp buffer.cpp tokenize/tokenize_c_re2c.cpp buffer.h
	$(CXX) $(CXXFLAGS) \
		alster_re2c.cpp \
		buffer.cpp \
		tokenize/tokenize_c_re2c.cpp \
		$(LDFLAGS) -o $@

fuzzer: alster.cpp buffer.cpp tokenize/tokenize_c.cpp buffer.h
	clang++ -DFUZZ -std=c++17 -g -O1 \
		-fsanitize=fuzzer,address alster.cpp \
		buffer.cpp tokenize/tokenize_c.cpp $(LDFLAGS) -o $@

%_re2c.cpp: %.cpp
	re2c -i $< -o $@
