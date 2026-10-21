# Builds OpenAudioBeBoBMAFW.kext with the command-line toolchain (Xcode 9.4.1, 10.13 SDK) -- no .xcodeproj.
# Deviation from BUILD-MANUAL §1 (which lists an .xcodeproj): a Makefile keeps the kext flags explicit and
# reviewable; the build is the same clang/ld invocation Xcode would make.
SDK    ?= $(shell xcrun --sdk macosx --show-sdk-path)
CXX    := $(shell xcrun -f clang++)
KH     := $(SDK)/System/Library/Frameworks/Kernel.framework
CCKEXT := $(shell find $(dir $(CXX))../lib/clang -name libclang_rt.cc_kext.a | head -1)
NAME   := OpenAudioBeBoBMAFW
OUT    ?= build
BUNDLE := $(OUT)/$(NAME).kext
# Modular tree (plan v4 AP-2.5): src/base = everything shared, src/models/<Model> = one folder per model, built on top of the base.
MODELS := $(wildcard src/models/*)
SRCS   := $(wildcard src/base/*.cpp) $(foreach m,$(MODELS),$(wildcard $(m)/*.cpp))
HDRS   := $(wildcard src/base/*.h) $(foreach m,$(MODELS),$(wildcard $(m)/*.h))
OBJS   := $(patsubst src/%.cpp,$(OUT)/obj/%.o,$(SRCS))

CXXFLAGS := -arch x86_64 -mmacosx-version-min=10.13 -isysroot $(SDK) \
  -std=gnu++11 -fno-rtti -fno-exceptions -fno-builtin -fno-common -fapple-kext -mkernel -static \
  -nostdinc -nostdinc++ -I$(KH)/Headers -I$(KH)/PrivateHeaders -Isrc/base $(foreach m,$(MODELS),-I$(m)) \
  -DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT -DMAC_OS_X_VERSION_MIN_REQUIRED=101300 \
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
