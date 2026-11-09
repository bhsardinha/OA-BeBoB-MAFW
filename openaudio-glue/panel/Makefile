# OpenAudio Glue app (GLUE_PLAN_v1.md phase 4). Built on the build PC: Xcode 9.4.1 / Swift 4.1 / 10.13 SDK.
#   make typecheck   type-check against the 10.13 SDK
#   make test        host tests of the pure logic (EQ maths vs the C++, settings plist, Dual routing)
#   make app         build/OpenAudio Glue.app (Swift runtime bundled: 10.13 has none)
# The look comes from the main panel's style sources, compiled unmodified (MAFW = the openaudio-mafw-bebob folder; on the build PC
# make MAFW=$HOME/oa-build-mafw).
MAFW   ?= ../../openaudio-mafw-bebob
SDK    := $(shell xcrun --sdk macosx --show-sdk-path)
SWIFTC := xcrun swiftc -sdk $(SDK) -target x86_64-apple-macosx10.13
STYLE  := $(MAFW)/control-panel/Sources/Theme.swift $(MAFW)/control-panel/Sources/Widgets.swift \
          $(MAFW)/control-panel/Sources/FaderLaw.swift $(MAFW)/control-panel/Sources/MaKeys.swift
SRCS   := $(wildcard Sources/*.swift)
LOGIC  := Sources/GlueModel.swift Sources/GlueEQMath.swift
BUILD  := build
APP    := $(BUILD)/OpenAudio Glue.app

.PHONY: typecheck test app clean
typecheck:
	$(SWIFTC) -typecheck $(STYLE) $(SRCS)

test: $(BUILD)/glue-tests
	$(BUILD)/glue-tests

$(BUILD)/glue-tests: $(LOGIC) Tests/GlueTests.swift
	mkdir -p $(BUILD)/t && cp Tests/GlueTests.swift $(BUILD)/t/main.swift
	$(SWIFTC) -o $@ $(LOGIC) $(BUILD)/t/main.swift

app:
	mkdir -p "$(APP)/Contents/MacOS"
	$(SWIFTC) -O -o "$(APP)/Contents/MacOS/OpenAudio Glue" $(STYLE) $(SRCS)
	cp Info.plist "$(APP)/Contents/Info.plist"
	mkdir -p "$(APP)/Contents/Resources" && cp Resources/AppIcon.icns "$(APP)/Contents/Resources/"   # made by Resources/make-icon.py
	$(MAFW)/scripts/bundle-swift-runtime.sh "$(APP)" "OpenAudio Glue"

clean:
	rm -rf $(BUILD)
