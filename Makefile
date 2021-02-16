CXXFLAGS=-std=c++17 -Wall -Wfatal-errors -O2 -g
LDFLAGS=-lncurses

alster: alster_re2c.cpp buffer.cpp tokenize_re2c.cpp buffer.h syntax/c.re2c
	$(CXX) $(CXXFLAGS) \
		alster_re2c.cpp \
		buffer.cpp \
		tokenize_re2c.cpp \
		$(LDFLAGS) -o $@

fuzzer: alster.cpp buffer.cpp tokenize_c.cpp buffer.h
	clang++ -DFUZZ -std=c++17 -g -O1 \
		-fsanitize=fuzzer,address alster.cpp \
		buffer.cpp tokenize.cpp $(LDFLAGS) -o $@

%_re2c.cpp: %.cpp syntax/c.re2c
	re2c -i $< -o $@
