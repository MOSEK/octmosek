
PKG_FILES = COPYING DESCRIPTION $(wildcard INDEX) $(wildcard PKG_ADD) \
        $(wildcard PKG_DEL) $(wildcard post_install.m) \
        $(wildcard pre_install.m)  $(wildcard on_uninstall.m) \
        $(wildcard inst/*) $(wildcard src/*) $(wildcard doc/*) \
        $(wildcard bin/*)

pre-pkg/%::
        # Do nothing prior to packaging

post-pkg/%::
        # Do nothing post packaging
