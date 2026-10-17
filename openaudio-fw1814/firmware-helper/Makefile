CC ?= clang
SRCS := main.c Gating.c Settings.c
all: build/oa-fwhelper
build/oa-fwhelper: $(SRCS) helper.h
	@mkdir -p build
	$(CC) -mmacosx-version-min=10.13 -Wall -Wextra -O2 -o $@ $(SRCS) -framework IOKit -framework CoreFoundation -framework SystemConfiguration
clean:
	rm -rf build
