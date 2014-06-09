.PHONY: build clean all install

all: build install

clean:
	rm -rf build

build:
	pebble build

install:
	PEBBLE_PHONE=${PEBBLE_PHONE} pebble install