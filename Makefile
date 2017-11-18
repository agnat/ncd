root_dir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
tap := $(root_dir)/node_modules/.bin/tap

node_modules:
	npm install

test: node_modules
	@$(tap) -b $(root_dir)/tests/*.js || exit 1 ;\
	for pool_size in 8 16 32 64 128 ; do \
		UV_THREADPOOL_SIZE="$$pool_size" $(tap) -b $(root_dir)/tests/*.js || exit 1 ; \
	done

clean:
	@find $(root_dir) \
			-path $(root_dir)/node_modules -prune -o \
			-name build -exec rm -r {} +

distclean: clean
	@rm -fr $(root_dir)/node_modules

.PHONY: test clean distclean
