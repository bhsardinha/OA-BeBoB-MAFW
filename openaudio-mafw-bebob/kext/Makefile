# Builds OpenAudioBeBoBMAFW.kext with the command-line toolchain (Xcode 9.4.1, 10.13 SDK) -- no .xcodeproj.
# Deviation from BUILD-MANUAL §1 (which lists an .xcodeproj): a Makefile keeps the kext flags explicit and
# reviewable; the build is the same clang/ld invocation Xcode would make.
# BAND=classic (default, 10.9-10.15, 10.13 SDK) | modern (11-15, 15.x SDK) | legacy (10.6.8-10.8.5, not built yet): the three editions of
# BROAD_COMPATIBILITY_PLAN_v2 (option B). Modern, BC-2.1/BC-3.3: macOS 14 Intel. The kernel's IOAudioFamily 540 replaced IOAudioEngine's reserved
# vtable slots 15/16 with calculateSampleTimeoutHiRes/useHiResSampleInterval, so a kext built with the 10.13 SDK imports two symbols
# 14 does not define (evidence/kext-0.2.2-kmutil-libraries-macos-14.8.9.txt); building against the 15.x SDK headers resolves them.
BAND   ?= classic
SDK    ?= $(shell xcrun --sdk macosx --show-sdk-path)
BANDDEF := $(if $(filter modern,$(BAND)),-DOA_BAND_MODERN)
# Minimum OS stays 10.13 for every band: the 15.x SDK's libkmod start/stop objects are stamped 15.2, so ld warns "built for newer macOS version"
# for any minimum below 15.2 (11.0 tried 2026-10-21: same warnings, identical imports, R128); harmless, the Modern kext ran on 14.8.9 like this.
CXX    := $(shell xcrun -f clang++)
KH     := $(SDK)/System/Library/Frameworks/Kernel.framework
CCKEXT := $(shell find $(dir $(CXX))../lib/clang -name libclang_rt.cc_kext.a | head -1)
NAME   := OpenAudioBeBoBMAFW
OUT    ?= $(if $(filter classic,$(BAND)),build,build/$(BAND))
BUNDLE := $(OUT)/$(NAME).kext
# Modular tree (plan v4 AP-2.5): src/base = everything shared, src/models/<Model> = one folder per model, built on top of the base.
MODELS := $(wildcard src/models/*)
SRCS   := $(wildcard src/base/*.cpp) $(foreach m,$(MODELS),$(wildcard $(m)/*.cpp))
HDRS   := $(wildcard src/base/*.h) $(foreach m,$(MODELS),$(wildcard $(m)/*.h))
OBJS   := $(patsubst src/%.cpp,$(OUT)/obj/%.o,$(SRCS))

CXXFLAGS := -arch x86_64 -mmacosx-version-min=10.13 -isysroot $(SDK) \
  -std=gnu++11 -fno-rtti -fno-exceptions -fno-builtin -fno-common -fapple-kext -mkernel -static \
  -nostdinc -nostdinc++ -I$(KH)/Headers -I$(KH)/PrivateHeaders -Isrc/base $(foreach m,$(MODELS),-I$(m)) \
  -DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT -DMAC_OS_X_VERSION_MIN_REQUIRED=101300 $(BANDDEF) \
  -Wall -Wextra -Wno-unused-parameter -Wno-deprecated-declarations -Wno-inconsistent-missing-override -O2 -g

all: $(BUNDLE)

$(OUT)/obj/%.o: src/%.cpp $(HDRS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUNDLE): $(OBJS) Info.plist
	@mkdir -p $(BUNDLE)/Contents/MacOS
	$(CXX) -arch x86_64 -mmacosx-version-min=10.13 -isysroot $(SDK) -nostdlib -Xlinker -kext \
	  -o $(BUNDLE)/Contents/MacOS/$(NAME) $(OBJS) -L$(SDK)/usr/lib -lkmodc++ -lkmod $(CCKEXT)
	cp Info.plist $(BUNDLE)/Contents/Info.plist
	plutil -lint $(BUNDLE)/Contents/Info.plist

clean:
	rm -rf $(OUT)
.PHONY: all clean
