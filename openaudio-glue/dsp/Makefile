# OpenAudio Glue - DSP chain. Must build on the build PC (10.13.6, Xcode 9.4.1): C++17 language, C++14 library (G2).
#   make            -> build/libgluedsp.a
#   make test       -> build/glue-tests (runs it)
#   make bench      -> build/glue-bench (runs it; see ../tools/README.md)
#   make listen     -> build/glue-listen (plays a file through the chain; see ../tools/README.md)
CXX      ?= clang++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -arch x86_64 -mmacosx-version-min=10.13
SRC      = AirwindowsMono.cpp GlueEQ.cpp GlueComp.cpp GluePresets.cpp GlueChain.cpp
OBJ      = $(SRC:%.cpp=build/%.o)
HDR      = Glue.h AirwindowsMono.h GlueEQ.h GlueComp.h

all: build/libgluedsp.a

build/%.o: %.cpp $(HDR)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/libgluedsp.a: $(OBJ)
	rm -f $@
	ar rcs $@ $(OBJ)

build/glue-tests: tests/glue_tests.cpp build/libgluedsp.a
	$(CXX) $(CXXFLAGS) $< build/libgluedsp.a -o $@

build/glue-bench: ../tools/glue-bench/glue_bench.cpp build/libgluedsp.a
	$(CXX) $(CXXFLAGS) -I. $< build/libgluedsp.a -o $@

build/glue-listen: ../tools/glue-listen/glue_listen.cpp build/libgluedsp.a
	$(CXX) $(CXXFLAGS) -I. $< build/libgluedsp.a -framework AudioToolbox -framework CoreAudio -framework CoreFoundation -o $@

listen: build/glue-listen

test: build/glue-tests
	./build/glue-tests

bench: build/glue-bench
	./build/glue-bench

clean:
	rm -rf build

.PHONY: all test bench listen clean
