# Panel build (Xcode 9.4.1 / Swift 4.1 / 10.13 SDK on the build PC). Targets:
#   make typecheck   type-check every source against the 10.13 SDK
#   make app         build build/OpenAudio FW1814.app (unsigned; signing is IN.6, user-owned)
#   make test        build and run the host unit test of the pure logic (MaKeys, ChannelLayout, DeviceModel, peak math)
# The dev Mac toolchain is not used for panel builds (its CLT modules do not match the 10.13 target).
SDK    := $(shell xcrun --sdk macosx --show-sdk-path)
SWIFTC := xcrun swiftc -sdk $(SDK) -target x86_64-apple-macosx10.13
SRCS   := $(filter-out Sources/main.swift,$(wildcard Sources/*.swift))
BUILD  := build

.PHONY: typecheck test app clean
typecheck:
	$(SWIFTC) -typecheck $(wildcard Sources/*.swift)

test: $(BUILD)/layout-tests
	$(BUILD)/layout-tests

LOGIC := Sources/MaKeys.swift Sources/ChannelLayout.swift Sources/DeviceModel.swift Sources/IOKitBridge.swift Sources/FirmwareInfo.swift

$(BUILD)/layout-tests: $(LOGIC) Tests/LayoutTests.swift
	mkdir -p $(BUILD)/t && cp Tests/LayoutTests.swift $(BUILD)/t/main.swift
	$(SWIFTC) -o $@ $(LOGIC) $(BUILD)/t/main.swift

APP := $(BUILD)/OpenAudio FW1814.app
app:
	mkdir -p "$(APP)/Contents/MacOS"
	$(SWIFTC) -O -o "$(APP)/Contents/MacOS/OpenAudio FW1814" $(wildcard Sources/*.swift)
	cp Info.plist "$(APP)/Contents/Info.plist"
	mkdir -p "$(APP)/Contents/Resources" && cp Resources/* "$(APP)/Contents/Resources/"
	../scripts/bundle-swift-runtime.sh "$(APP)" "OpenAudio FW1814"        # 10.13 has no Swift runtime: ship it inside the app

clean:
	rm -rf $(BUILD)
