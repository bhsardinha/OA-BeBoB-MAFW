# Panel build (Xcode 9.4.1 / Swift 4.1 / 10.13 SDK on the build PC). Targets:
#   make typecheck   type-check every source against the 10.13 SDK
#   make app         build build/OA BeBoB MAFW.app (with the firmware check window, formerly the separate Firmware Tool) (unsigned; signing is IN.6, user-owned)
#                    BAND=modern|legacy: build/<band>/OA BeBoB MAFW.app; Info.plist OAEdition = the band (default classic; Sources/Edition.swift)
#   make test        build and run the host unit test of the pure logic (MaKeys, ChannelLayout, DeviceModel, peak math)
# The dev Mac toolchain is not used for panel builds (its CLT modules do not match the 10.13 target).
SDK    := $(shell xcrun --sdk macosx --show-sdk-path)
SWIFTC := xcrun swiftc -sdk $(SDK) -target x86_64-apple-macosx10.13
# Modular sources (AP-5.1): Sources/ = generic panel, Sources/Models/<Model>/ = one folder per model
ALLSRC := $(wildcard Sources/*.swift) $(wildcard Sources/Models/*/*.swift)
SRCS   := $(filter-out Sources/main.swift,$(ALLSRC))
BUILD  := build
# Edition (BROAD_COMPATIBILITY_PLAN_v2 §11a), the kext Makefile's names: only the About tab and the app's folder depend on it
BAND   ?= classic
APPDIR := $(if $(filter classic,$(BAND)),$(BUILD),$(BUILD)/$(BAND))

.PHONY: typecheck test app clean
typecheck:
	$(SWIFTC) -typecheck $(ALLSRC)

test: $(BUILD)/layout-tests $(BUILD)/firmware-tests
	$(BUILD)/layout-tests
	$(BUILD)/firmware-tests

# firmware check (formerly the separate Firmware Tool, merged into the panel 2026-10-21): catalog and verdicts
$(BUILD)/firmware-tests: Sources/FirmwareCatalog.swift Sources/FirmwareInfo.swift Tests/FirmwareCatalogTests.swift
	mkdir -p $(BUILD)/ft && cp Tests/FirmwareCatalogTests.swift $(BUILD)/ft/main.swift
	$(SWIFTC) -o $@ Sources/FirmwareCatalog.swift Sources/FirmwareInfo.swift $(BUILD)/ft/main.swift

LOGIC := Sources/Edition.swift Sources/FaderLaw.swift Sources/StatusLogic.swift Sources/MaKeys.swift Sources/ChannelLayout.swift Sources/Models/FW1814/FW1814Layout.swift Sources/Translator.swift Sources/DeviceModel.swift Sources/FirmwareInfo.swift Sources/RateFollower.swift

$(BUILD)/layout-tests: $(LOGIC) Tests/LayoutTests.swift
	mkdir -p $(BUILD)/t && cp Tests/LayoutTests.swift $(BUILD)/t/main.swift
	$(SWIFTC) -o $@ $(LOGIC) $(BUILD)/t/main.swift

APP := $(APPDIR)/OA BeBoB MAFW.app
app:
	mkdir -p "$(APP)/Contents/MacOS"
	$(SWIFTC) -O -o "$(APP)/Contents/MacOS/OA BeBoB MAFW" $(ALLSRC)
	cp Info.plist "$(APP)/Contents/Info.plist"
	/usr/libexec/PlistBuddy -c "Add :OAPackageVersion string $$(cat ../VERSION 2>/dev/null || echo -)" "$(APP)/Contents/Info.plist"
	/usr/libexec/PlistBuddy -c "Add :OAEdition string $(BAND)" "$(APP)/Contents/Info.plist"
	mkdir -p "$(APP)/Contents/Resources" && cp Resources/* "$(APP)/Contents/Resources/"
	../scripts/bundle-swift-runtime.sh "$(APP)" "OA BeBoB MAFW"        # 10.13 has no Swift runtime: ship it inside the app

clean:
	rm -rf $(BUILD)
