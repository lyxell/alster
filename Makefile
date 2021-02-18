CXXFLAGS=-std=c++17 \
		 -Wall \
		 -flto \
		 -O2 \
		 -Wfatal-errors \
		 -Wconversion \
		 -Wshadow \
		 -Wswitch-default \
		 -Wdisabled-optimization \
		 -march=native \
		 -g

build/alster: build/alster_re2c.cpp buffer.cpp \
			  buffer.h syntax/c.re2c build/window_re2c.cpp file.cpp file.h utf8.cpp
			 
	mkdir -p build
	$(CXX) \
		$(CXXFLAGS) \
		-I. \
		build/alster_re2c.cpp \
		buffer.cpp \
		utf8.cpp \
		build/window_re2c.cpp \
		file.cpp \
		 -o $@

build/fuzzer: build/alster_re2c.cpp buffer.cpp \
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
		 -o $@

build/%_re2c.cpp: %.cpp syntax/c.re2c
	mkdir -p build
	re2c -W -i $< -o $@

.PHONY: clean
clean:
	rm -rf build
