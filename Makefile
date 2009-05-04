include Makefile.global

SSCLC_LIB = libssclc.so.$(VERSION)
SSCL_AR = libsscl.a
SSCL_LIB = libsscl.so.$(VERSION)

SSCLC_SRC = $(wildcard ssclc/*.c)
SSCL_SRC = $(wildcard sscl/*.cc)

SSCLC_OBJ = $(SSCLC_SRC:ssclc/%.c=build/ssclc/%.o)
SSCL_OBJ = $(SSCL_SRC:sscl/%.cc=build/sscl/%.o)
SSCL_GITID = $(shell cat "$$( (if [ -L ".git/HEAD" ];then readlink .git/HEAD;else cat .git/HEAD 2>/dev/null; fi; ) | sed 's+.*refs/+.git/refs/+')" 2>/dev/null)

SSCLC_OBJ := $(SSCLC_OBJ) build/ssclc/gitid.o
SSCL_OBJ := $(SSCL_OBJ) build/ssclc/gitid.o
ifneq (,$(SSCL_GITID))
	SSCL_GITID := -id-$(SSCL_GITID)
endif

#SSCLC_OBJ = ssclc/o/strfunc.o o/list.o o/avltree.o o/stream.o \
#	o/network.o o/lexical.o
#SSCL_OBJ = o/strfunc.o o/error.o o/object.o o/list.o o/avltree.o \
#	  o/stream.o o/network.o o/lexical.o \
#	  o/sdt.o

#---------------------------------------- Main rules
all: build $(SSCLC_LIB) $(SSCLC_SO_NAME) libssclc.so libssclc-$(MAIN_VERSION).so $(SSCL_AR) sscl.spec

clean:
	rm -rf build; rm -f core .depend libsscl.so* libssclc.so* libsscl.a

depend:
	gcc -M -MM $(CCINCLUDE) *.cc |sed 's/^/o\//' >.depend
	@echo "Dependencies were remade, type \`make' to make the binaries!"

install_root:
	#$(INSTALL) --mode 0755 $(SSCLC_LIB) $(SSCL_LIB) $(LIBDIR)
	$(INSTALL) --mode 0755 $(SSCLC_LIB) $(SSCL_AR) $(SSCLC_SO_NAME) libssclc.so libssclc-$(MAIN_VERSION).so $(LIBDIR)
#	(cd $(LIBDIR) && $(LN_S) -f $(SSCLC_LIB) $(SSCLC_SO_NAME))
#	(cd $(LIBDIR) && $(LN_S) -f $(SSCLC_LIB) libssclc.so)
	mkdir -p $(INCLUDEDIR)
	$(INSTALL) --mode 0644 include/sscl/*.h $(INCLUDEDIR)

install: all
	@if [ "`id -u`" != '0' ]; then echo "Root password is required.."; su -c '${MAKE}  install_root'; else ${MAKE} install_root; fi

uninstall_root:
	rm -f $(LIBDIR)/libssclc.so* $(LIBDIR)/libsscl.a
	rm -rf $(INCLUDEDIR)

uninstall:
	@if [ `id -u` != '0' ]; then echo "Root password is required.."; su -c '${MAKE}  uninstall_root'; else ${MAKE} install_root; fi

rpm_root: install sscl.spec
	rpm -bb sscl.spec

rpm: sscl.spec
	@if [ `id -u` != '0' ]; then echo "Root password is required.."; su -c '${MAKE}  rpm_root'; else ${MAKE} rpm_root; fi

deb:
	dpkg-buildpackage -rfakeroot

ifneq (,$(wildcard .depend))
    include .depend
endif

build:
	@echo "[1mPreparing build environment..[0m"
	@mkdir build; mkdir build/ssclc build/sscl

#---------------------------------------- Template rules
$(SSCLC_LIB): $(SSCLC_OBJ)
	$(LD) $(CLDOPTS) -o $@ $(SSCLC_OBJ)

$(SSCL_AR): $(SSCL_OBJ)
	ar cru $@ $(SSCL_OBJ)
	ranlib $@
$(SSCL_LIB): $(SSCL_OBJ)
	ld $(CCLDOPTS) -o $@ $(SSCL_OBJ)

$(SSCLC_SO_NAME):
	$(LN_S) -f $(SSCLC_LIB) $@
libssclc-$(MAIN_VERSION).so:
	$(LN_S) -f $(SSCLC_LIB) $@
libssclc.so:
	$(LN_S) -f $(SSCLC_LIB) $@
#libsscl.so:
#	$(LN_S) -f $(TARGET) libsscl.so
#$(SO_NAME):
#	$(LN_S) -f $(TARGET) $(SO_NAME)

build/ssclc/%.o: ssclc/%.c
	$(CC) $(COPTS) -c -o $@ $<

build/sscl/%.o: sscl/%.cc
	$(CC) $(CCOPTS) -c -o $@ $<

build/ssclc/gitid.o: build/gitid.c
	$(CC) $(COPTS) -c -o $@ $<

build/gitid.c: Makefile
	echo 'char *_sscl_version_id="SSCL_VERSION-'$(VERSION)$(SSCL_GITID)'";' > $@

sscl.spec: sscl.spec.in
	sed -e "s/_VER_/$(VERSION)/" -e "s!_PREFIX_!$(PREFIX)!" <sscl.spec.in >sscl.spec
