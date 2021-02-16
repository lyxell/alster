CXXFLAGS=-std=c++17 -Wall -Wfatal-errors -O2 -g
LDFLAGS=-lncurses

build/alster: build/alster_re2c.cpp buffer.cpp build/tokenize_re2c.cpp \
			  buffer.h syntax/c.re2c
	mkdir -p build
	$(CXX) $(CXXFLAGS) \
		-I. \
		build/alster_re2c.cpp \
		buffer.cpp \
		build/tokenize_re2c.cpp \
		$(LDFLAGS) -o $@

build/fuzzer: alster.cpp buffer.cpp tokenize_c.cpp buffer.h
	mkdir -p build
	clang++ -DFUZZ -std=c++17 -g -O1 \
		-fsanitize=fuzzer,address alster.cpp \
		buffer.cpp tokenize.cpp $(LDFLAGS) -o $@

build/%_re2c.cpp: %.cpp syntax/c.re2c
	mkdir -p build
	re2c -i $< -o $@

.PHONY: clean
clean:
	rm -rf build
