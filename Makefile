root_dir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
tap := $(root_dir)/node_modules/.bin/tap
eslint := $(root_dir)/node_modules/.bin/eslint
clang_tidy := /usr/local/opt/llvm/bin/clang-tidy
headers := $(shell find $(root_dir) -name "*.hpp")
node_version = $(shell node -v | cut -c 2-)

node_modules:
	@npm install

testrun: node_modules
	@$(tap) -b $(root_dir)/tests/*.js || exit 1; \
	for pool_size in 8 16 32 64 128 ; do \
		UV_THREADPOOL_SIZE="$$pool_size" $(tap) -b $(root_dir)/tests/*.js || exit 1; \
	done

lint: node_modules
	@$(eslint) $(root_dir)/lib/*.js; \
	$(clang_tidy) $(headers) -- \
			-I include \
			-I $(HOME)/.node-gyp/$(node_version)/include/node \
			-I $(root_dir)/node_modules/nan \
			-std=c++11

test: testrun lint

clean:
	@find $(root_dir) \
			-path $(root_dir)/node_modules -prune -o \
			-name build -exec rm -r {} +

distclean: clean
	@rm -fr $(root_dir)/node_modules

.PHONY: test testrun lint clean distclean
