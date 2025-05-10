ARDUINO_CLI ?= arduino-cli
PORT ?= /dev/ttyACM0

SRC_FILES := $(wildcard src/*.cpp)
HEADER_FILES := $(wildcard src/*.h)
BUILD_FILES := $(patsubst src/%.cpp, _build/%.cpp, $(SRC_FILES)) $(patsubst src/%.h, _build/%.h, $(HEADER_FILES))

upload: compile
	@$(ARDUINO_CLI) upload -p $(PORT) --fqbn arduino:mbed_opta:opta _build

compile: copy
	@$(ARDUINO_CLI) compile --fqbn arduino:mbed_opta:opta _build


copy: $(BUILD_FILES) _build/_build.ino


_build/%.cpp: src/%.cpp
	@mkdir -p $(dir $@)
	cp $< $@

_build/%.h: src/%.h
	@mkdir -p $(dir $@)
	cp $< $@

_build/_build.ino:
	@mkdir -p _build
	@echo "void setup(); void loop();" > _build/_build.ino


clean:
	@rm -rf _build

tidy:
	clang-format -i $(SRC_FILES) $(HEADER_FILES)


.PHONY: copy compile upload clean