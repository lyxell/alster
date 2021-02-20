CXXFLAGS=-std=c++17 \
		 -static \
		 -Wall \
		 -g \
		 -O2 \
		 -Wfatal-errors \
		 -Wconversion \
		 -Wshadow \
		 -Werror \
		 -Wdisabled-optimization

build/alster: alster.cpp buffer.cpp build/syntax/c_re2c.cpp build/syntax/c_string_re2c.cpp tty.cpp build/editor_re2c.cpp \
			  buffer.h build/window_re2c.cpp file.cpp file.h utf8.cpp
			 
	mkdir -p build
	$(CXX) \
		$(CXXFLAGS) \
		-I. \
		-Isyntax \
		alster.cpp \
		buffer.cpp \
		utf8.cpp \
		build/editor_re2c.cpp \
		tty.cpp \
		build/syntax/c_re2c.cpp \
		build/syntax/c_string_re2c.cpp \
		build/window_re2c.cpp \
		file.cpp \
		 -o $@

build/test: test/main.cpp build/editor_re2c.cpp utf8.cpp buffer.cpp window.cpp build/syntax/c_re2c.cpp build/syntax/c_string_re2c.cpp
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
	re2c -W --input-encoding utf8 -i $< -o $@

.PHONY: clean
clean:
	rm -rf build
