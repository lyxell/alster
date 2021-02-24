CXXFLAGS=-std=c++17 \
		 -g \
		 -O2 \
		 -Wall \
		 -Wshadow \
		 -Wconversion \
		 -Wfatal-errors \
		 -Werror

build/alster: \
	build/alster.o \
	build/buffer.o \
	buffer.h \
	build/editor_re2c.o \
	build/syntax/c_re2c.o \
	build/syntax/c_string_re2c.o \
	build/window_re2c.o \
	file.o \
	file.h \
	tokenize.h \
	tty.o \
	utf8.o
	mkdir -p build
	$(CXX) \
		build/alster.o \
		build/buffer.o \
		utf8.o \
		build/editor_re2c.o \
		tty.o \
		build/syntax/c_re2c.o \
		build/syntax/c_string_re2c.o \
		build/window_re2c.o \
		file.o \
		 -o $@

build/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I. $< -c -o $@

build/%.o: build/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I. $< -c -o $@

build/%_re2c.cpp: %.cpp
	mkdir -p $(@D)
	re2c -W --input-encoding utf8 -i $< -o $@

.PHONY: clean test
clean:
	rm -rf build
test: build/test
	./build/test
