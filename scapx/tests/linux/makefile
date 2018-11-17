.PHONY: all
all: scapx_test_static scapx_test_shared

.PHONY: rebuild
rebuild: clean scapx_test_static scapx_test_shared

.PHONY: scapx_test_static
scapx_test_static:
	make -f makefile.static scapx_test_static

.PHONY: scapx_test_shared
scapx_test_shared:
	make -f makefile.shared scapx_test_shared

.PHONY: clean
clean:
	make -f makefile.static clean
	make -f makefile.shared clean

# If an error occurrs during a target's recipe, delete the target file if
# it was already created.
.DELETE_ON_ERROR:
