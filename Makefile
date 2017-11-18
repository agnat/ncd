root_dir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
tap := $(root_dir)/node_modules/.bin/tap

distclean: clean
	rm -fr $(root_dir)/node_modules

clean:
	find $(root_dir) \
			-path $(root_dir)/node_modules -prune -o \
			-name build -print -exec rm -r {} +

test:
	$(tap) -b $(root_dir)/tests/*.js ;\
	for pool_size in 8 16 32 64 128 ; do \
		UV_THREADPOOL_SIZE="$$pool_size" $(tap) -b $(root_dir)/tests/*.js ; \
	done

.PHONY: clean distclean test
