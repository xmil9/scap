.PHONY: all
all: libscapx_static libscapx_shared

.PHONY: rebuild
rebuild: clean libscapx_static libscapx_shared

.PHONY: libscapx_static
libscapx_static:
	make -f makefile.static libscapx.a

.PHONY: libscapx_shared
libscapx_shared:
	make -f makefile.shared libscapx.so

.PHONY: clean
clean:
	make -f makefile.static clean
	make -f makefile.shared clean

.PHONY: install
install:
	make -f makefile.static install
	make -f makefile.shared install

.PHONY: uninstall
uninstall:
	make -f makefile.static uninstall
	make -f makefile.shared uninstall

# If an error occurrs during a target's recipe, delete the target file if
# it was already created.
.DELETE_ON_ERROR:
