include Makefile.global

TARGET = libsscl.so.$(VERSION)
OBJECTS = o/strfunc.o o/error.o o/object.o o/list.o o/avltree.o \
	  o/stream.o o/network.o o/lexical.o \
	  o/sdt.o

#---------------------------------------- Main rules
all: o $(TARGET) $(SO_NAME) libsscl.so libsscl.a sscl.ver sscl.spec

clean:
	rm -rf o; rm -f core .depend $(TARGET) $(SO_NAME) libsscl.so libsscl.a

depend:
	gcc -M -MM $(CCINCLUDE) *.cc |sed 's/^/o\//' >.depend
	@echo "Dependencies were remade, type \`make' to make the binaries!"

install_root:
	$(INSTALL) --mode 0755 $(TARGET) libsscl.a $(LIBDIR)
	(cd $(LIBDIR) && $(LN_S) -f $(TARGET) $(SO_NAME))
	(cd $(LIBDIR) && $(LN_S) -f $(TARGET) libsscl.so)
	mkdir -p $(INCLUDEDIR)
	$(INSTALL) --mode 0644 include/sscl/*.h $(INCLUDEDIR)

install: all
	@if [ $$UID != '0' ]; then echo "Root password is required.."; su -c '${MAKE}  install_root'; else ${MAKE} install_root; fi

rpm_root: install sscl.spec
	rpm -bb sscl.spec

rpm: sscl.spec
	@if [ $$UID != '0' ]; then echo "Root password is required.."; su -c '${MAKE}  rpm_root'; else ${MAKE} rpm_root; fi

ifneq (,$(wildcard .depend))
    include .depend
endif

o:
	mkdir $@

#---------------------------------------- Template rules
$(TARGET): $(OBJECTS)
	ld $(LDOPTS) -o $@ $(OBJECTS)
#	$(LD) $(LDOPTS) -o $@ $<

libsscl.a: $(OBJECTS)
	ar cru $@ $(OBJECTS)

o/%.o: %.cc
	$(CC) $(CCOPTS) -c -o $@ $<

$(SO_NAME):
	$(LN_S) -f $(TARGET) $(SO_NAME)

libsscl.so:
	$(LN_S) -f $(TARGET) libsscl.so

sscl.ver:
	echo "$(VERSION)" >$@

sscl.spec: sscl.spec.in
	sed -e "s/_VER_/$(VERSION)/" -e "s!_PREFIX_!$(PREFIX)!" <sscl.spec.in >sscl.spec
