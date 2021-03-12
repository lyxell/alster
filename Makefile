CXXFLAGS=-std=c++17 \
		 -g \
		 -O2 \
		 -Wall \
		 -Wshadow \
		 -Wconversion \
		 -Wfatal-errors

CFLAGS=-std=c99 -g -O2 -Wall

.SUFFIXES:

build/alster: \
	build/alster.o \
	build/editor.o \
	build/syntax/c_re2c.o \
	build/window_re2c.o \
	build/tty.o \
	build/lua.o \
	build/utf8.o
	@mkdir -p build
	$(CXX) \
		build/alster.o \
		build/utf8.o \
		build/editor.o \
		build/tty.o \
		build/lua.o \
		build/syntax/c_re2c.o \
		build/window_re2c.o \
		 -o $@ -llua5.1

build/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I. $< -c -o $@

build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -I. $< -c -o $@

build/%.o: build/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -I. $< -c -o $@

build/%.o: build/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I. $< -c -o $@

build/%_re2c.cpp: %.cpp
	@mkdir -p $(@D)
	re2c -W --input-encoding utf8 -i $< -o $@

build/%_re2c.c: %.c
	@mkdir -p $(@D)
	re2c -W --input-encoding utf8 -i $< -o $@

.PHONY: clean test
clean:
	rm -rf build
test: build/test
	./build/test
