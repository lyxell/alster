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

build/alster: build/alster_re2c.cpp buffer.cpp build/syntax/c_re2c.cpp tty.cpp \
			  buffer.h build/window_re2c.cpp file.cpp file.h utf8.cpp
			 
	mkdir -p build
	$(CXX) \
		$(CXXFLAGS) \
		-I. \
		-Isyntax \
		build/alster_re2c.cpp \
		buffer.cpp \
		utf8.cpp \
		tty.cpp \
		build/syntax/c_re2c.cpp \
		build/syntax/c_string_re2c.cpp \
		build/window_re2c.cpp \
		file.cpp \
		 -o $@

build/test_tokenize: test/test_tokenize.cpp build/syntax/c_re2c.cpp build/syntax/c_string_re2c.cpp utf8.cpp
	$(CXX) $(CXXFLAGS) $^ -I. -Isyntax -o $@

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

build/%_re2c.cpp: %.cpp
	mkdir -p $(@D)
	re2c -W -i $< -o $@

.PHONY: clean
clean:
	rm -rf build
