# OpenAudio Glue - port-diff (see port_diff.cpp). Run ./fetch-ref.sh once first; `make` builds and runs.
CXX      ?= clang++
CXXFLAGS ?= -std=c++17 -O2 -arch x86_64 -mmacosx-version-min=10.13 -w
DSP      = ../../dsp
ORIGS    = Interstage Tube2 Channel9 ToTape9

run: build/port-diff
	./build/port-diff

define ORIG_RULE
build/lib$(1).o: originals.cpp ref/$(1)/$(1).cpp ref/$(1)/$(1)Proc.cpp vst-stub/audioeffectx.h
	@mkdir -p build
	$$(CXX) $$(CXXFLAGS) -Ivst-stub -Iref/$(1) -DORIG=$(1) -c originals.cpp -o build/wrap_$(1).o
	$$(CXX) $$(CXXFLAGS) -Ivst-stub -Iref/$(1) -DcreateEffectInstance=createEffectInstance_$(1) -c ref/$(1)/$(1).cpp -o build/plug_$(1).o
	$$(CXX) $$(CXXFLAGS) -Ivst-stub -Iref/$(1) -c ref/$(1)/$(1)Proc.cpp -o build/proc_$(1).o
	ld -r build/wrap_$(1).o build/plug_$(1).o build/proc_$(1).o -o $$@
endef
$(foreach o,$(ORIGS),$(eval $(call ORIG_RULE,$(o))))

build/port-diff: port_diff.cpp $(ORIGS:%=build/lib%.o) $(DSP)/build/libgluedsp.a
	$(CXX) $(CXXFLAGS) -I$(DSP) port_diff.cpp $(ORIGS:%=build/lib%.o) $(DSP)/build/libgluedsp.a -o $@

$(DSP)/build/libgluedsp.a:
	$(MAKE) -C $(DSP)

clean:
	rm -rf build

.PHONY: run clean
