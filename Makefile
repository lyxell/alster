CXXFLAGS=-std=c++17 -flto -Wall -Wfatal-errors -Wconversion -Wshadow -Wswitch-default -Wdisabled-optimization -O2 -march=native -g

build/alster: build/alster_re2c.cpp buffer.cpp build/tokenize_re2c.cpp \
			  buffer.h syntax/c.re2c window.cpp file.cpp file.h utf8.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) \
		-I. \
		build/alster_re2c.cpp \
		buffer.cpp \
		utf8.cpp \
		window.cpp \
		file.cpp \
		build/tokenize_re2c.cpp \
		 -o $@

build/fuzzer: build/alster_re2c.cpp buffer.cpp build/tokenize_re2c.cpp \
			  buffer.h syntax/c.re2c window.cpp
	mkdir -p build
	clang++ \
		-DFUZZ \
		-std=c++17 -g -O1 \
		-I. \
		build/alster_re2c.cpp \
		-fsanitize=fuzzer,address \
		buffer.cpp \
		window.cpp \
		file.cpp \
		build/tokenize_re2c.cpp \
		 -o $@

build/%_re2c.cpp: %.cpp syntax/c.re2c
	mkdir -p build
	re2c -W -i $< -o $@

.PHONY: clean
clean:
	rm -rf build
