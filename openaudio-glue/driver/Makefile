# OpenAudio Glue - the AudioServerPlugIn bundle build/OpenAudioGlue.driver. Built on the build PC (10.13.6, Xcode 9.4.1, 10.13 SDK):
# C++17 language, C++14 library (Glue G2). KEXT_INC is the folder with the kext's GlueTap.h (the contract, main R-log R143); on the
# build PC: make KEXT_INC=$HOME/oa-build-mafw/kext/src/base
CXX      ?= clang++
KEXT_INC ?= ../../openaudio-mafw-bebob/kext/src/base
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -arch x86_64 -mmacosx-version-min=10.13 -fvisibility=hidden -fvisibility-inlines-hidden
INC      = -Isrc -I../dsp -I$(KEXT_INC)
SRC      = src/GluePlugIn.cpp src/GlueDevice.cpp src/GlueTapClient.cpp src/GlueSettings.cpp src/GlueWorkers.cpp
OBJ      = $(SRC:src/%.cpp=build/obj/%.o)
BUNDLE   = build/OpenAudioGlue.driver
DSPLIB   = ../dsp/build/libgluedsp.a

all: $(BUNDLE)

build/obj/%.o: src/%.cpp src/*.h $(KEXT_INC)/GlueTap.h
	@mkdir -p build/obj
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(DSPLIB):
	$(MAKE) -C ../dsp

$(BUNDLE): $(OBJ) $(DSPLIB) Info.plist
	@mkdir -p $(BUNDLE)/Contents/MacOS
	cp Info.plist $(BUNDLE)/Contents/Info.plist
	$(CXX) $(CXXFLAGS) -bundle $(OBJ) $(DSPLIB) -framework CoreAudio -framework CoreFoundation -framework IOKit \
	    -o $(BUNDLE)/Contents/MacOS/OpenAudioGlue
	plutil -lint $(BUNDLE)/Contents/Info.plist
	@nm -gU $(BUNDLE)/Contents/MacOS/OpenAudioGlue | grep -q GlueDriverFactory && echo "factory exported: GlueDriverFactory"

# Host tests of the IO path against a simulated tap (no kext, no coreaudiod)
build/driver-tests: tests/driver_tests.cpp $(filter-out build/obj/GluePlugIn.o,$(OBJ)) $(DSPLIB)
	$(CXX) $(CXXFLAGS) $(INC) $< $(filter-out build/obj/GluePlugIn.o,$(OBJ)) $(DSPLIB) -framework CoreAudio -framework CoreFoundation -framework IOKit -o $@

test: build/driver-tests
	./build/driver-tests

clean:
	rm -rf build

.PHONY: all test clean
