CXXFLAGS=-std=c++17 \
		 -g \
		 -O2 \
		 -Wall \
		 -Wshadow \
		 -Wconversion \
		 -Wfatal-errors

build/alster: \
	build/alster.o \
	buffer.h \
	build/editor.o \
	build/syntax/c_re2c.o \
	build/syntax/c_string_re2c.o \
	build/window_re2c.o \
	build/unicode_re2c.o \
	build/file.o \
	file.h \
	build/tty.o \
	build/lua.o \
	build/utf8.o
	mkdir -p build
	$(CXX) \
		build/alster.o \
		build/lua.o \
		build/utf8.o \
		build/editor.o \
		build/tty.o \
		build/unicode_re2c.o \
		build/syntax/c_re2c.o \
		build/syntax/c_string_re2c.o \
		build/window_re2c.o \
		build/file.o \
		 -o $@ -llua5.1

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
