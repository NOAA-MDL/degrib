# $Id: Makefile,v 1.43 2004/01/27 14:08:14 ed Exp $


include macros.make


PACKAGE		= netcdf
SUBDIRS		= libsrc nctest nc_test fortran f90 nf_test ncdump ncgen man cxx
DIST_GARBAGE	= \
		config.cache	\
		config.status	\
		MANIFEST
PACKING_LIST	= \
		INSTALL.html	\
		COMPATIBILITY	\
		COPYRIGHT	\
		MANIFEST	\
		Makefile	\
		README		\
		RELEASE_NOTES	\
		VERSION		\
		aclocal.m4	\
		configure	\
		configure.in	\
		macros.make.in	\
		rules.make

# I moved fortran to the bottom of the list, sometimes AIX make get's confused
# if the C library and the fortran library are built in the same second. - Ed
all:		libsrc/all	\
		ncdump/all	\
		ncgen/all
#		cxx/all
#		fortran/all
#		f90/all

test:		fortran/all \
		libsrc/test \
		ncdump/test ncgen/test
#     cxx/test
#		nc_test/test nctest/test
#		nf_test/test fortran/test f90/test

check:		test

install:	libsrc/install \
		ncdump/install \
		ncgen/install
#		cxx/install
#		fortran/install
#		f90/install

uninstall:	libsrc/uninstall \
		ncdump/uninstall \
		ncgen/uninstall
#		cxx/uninstall
#		fortran/uninstall
#		f90/uninstall

clean:		libsrc/clean \
		ncdump/clean ncgen/clean
#     cxx/clean nctest/clean nc_test/clean
#		fortran/clean f90/clean nf_test/clean

distclean:	libsrc/distclean \
		ncdump/distclean ncgen/distclean
# cxx/distclean
#		nctest/distclean nc_test/distclean
#		fortran/distclean f90/distclean nf_test/distclean

libsrc/all	\
libsrc/test	\
libsrc/install	\
libsrc/uninstall	\
libsrc/clean	\
libsrc/distclean	\
nctest/all	\
nctest/test	\
nctest/clean	\
nctest/distclean	\
nc_test/all	\
nc_test/test	\
nc_test/clean	\
nc_test/distclean	\
fortran/clean	\
fortran/distclean	\
f90/clean	\
f90/distclean	\
nf_test/clean	\
nf_test/distclean	\
ncdump/all	\
ncdump/test	\
ncdump/install	\
ncdump/uninstall	\
ncdump/clean	\
ncdump/distclean	\
ncgen/all	\
ncgen/test	\
ncgen/install	\
ncgen/uninstall	\
ncgen/clean	\
ncgen/distclean	\
cxx/clean	\
cxx/distclean	\
:
	@subdir=`echo $@ | sed 's,/.*,,'`; \
	target=`echo $@ | sed 's,.*/,,'`; \
	$(MAKE) SUBDIR=$$subdir TGET=$$target subdir_target

fortran/all	\
fortran/test	\
fortran/install	\
fortran/uninstall	\
nf_test/all	\
nf_test/test	\
:
	@case "$(FC)" in \
	    '') echo 1>&2 "Not making \`$@' because no FORTRAN compiler"; \
		;; \
	    *)  subdir=`echo $@ | sed 's,/.*,,'`; \
		target=`echo $@ | sed 's,.*/,,'`; \
		$(MAKE) SUBDIR=$$subdir TGET=$$target subdir_target; \
		;; \
	esac

f90/all	\
f90/test	\
f90/install	\
f90/uninstall	\
:
	@case "$(F90)" in \
	    '') echo 1>&2 "Not making \`$@' because no FORTRAN-90 compiler"; \
		;; \
	    *)  subdir=`echo $@ | sed 's,/.*,,'`; \
		target=`echo $@ | sed 's,.*/,,'`; \
		$(MAKE) SUBDIR=$$subdir TGET=$$target subdir_target; \
		;; \
	esac

cxx/all	\
cxx/test	\
cxx/install	\
cxx/uninstall	\
:
	@case "$(CXX)" in \
	    '') echo 1>&2 "Not making \`$@' because no C++ compiler"; \
		;; \
	    *)  subdir=`echo $@ | sed 's,/.*,,'`; \
		target=`echo $@ | sed 's,.*/,,'`; \
		$(MAKE) SUBDIR=$$subdir TGET=$$target subdir_target; \
		;; \
	esac

subdir_target:
	@echo ""
	@cd $(SUBDIR) && \
	    echo "Making \`$(TGET)' in directory `pwd`" && \
	    echo "" && \
	    $(MAKE) $(TGET) || exit 1
	@echo ""
	@echo "Returning to directory `pwd`"
	@echo ""

install:	whatis
whatis:		$(MANDIR)/$(WHATIS)
$(MANDIR)/$(WHATIS):	$(MANDIR)
	$(MAKEWHATIS_CMD)
uninstall:
	rm -f $(MANDIR)/$(WHATIS)


################################################################################
# Distribution:

# The following rule checks to see that we're on the right system.  If we're
# not, then the distribution mightn't contain the intended ncgen(1) (not
# to mention that dvips(1) is necessary for building the C++ User's Guide).
#
check_system:	FORCE
	@case "$(OVERRIDE)" in \
	'') case `uname -sr` in \
	    'SunOS 5'*) \
		exit 0;; \
	    *)  echo 1>&2 "Error: Not on a SunOS 5 system."; \
		echo 1>&2 "Set macro OVERRIDE to override."; \
		exit 1;; \
	    esac;; \
	*) exit 0;; \
	esac

# Make a compressed, tar(1) file of the source distribution in the current 
# directory.
#
tar.Z:		check_system FORCE
	@version=`cat VERSION`; \
	    $(MAKE) $(MFLAGS) $(PACKAGE)-$$version.tar.Z VERSION=$$version

$(PACKAGE)-$(VERSION).tar.Z:	ensure_manifest MANIFEST
	id=$(PACKAGE)-$(VERSION) \
	&& rm -rf $$id \
	&& mkdir $$id \
	&& ln -s .. $$id/src \
	&& tar $(TARFLAGS) - `sed "s|^|$$id/src/|" MANIFEST` | compress > $@ \
	&& rm -r $$id

MANIFEST:	FORCE
	$(MAKE) MANIFEST.echo >$@

# Make a compressed, tar(1) file of the source distribution in the
# appropriate FTP directory.
#
# NOTE: Making "ftp" will cause the "tar.Z" file to be made even if it
# already exists.  This is because the MANIFEST file upon which it
# depends must be remade every time.  This is not a waste of time,
# however, if the "tar.Z" target is made in private directories and the
# "ftp" target is made in the "/upc/$(PACKAGE)/build/" directory.
#
ftp:		check_system FORCE
	version=`cat VERSION`; \
	$(MAKE) $(MFLAGS) $(FTPDIR)/$(PACKAGE)-$$version.tar.Z \
	    VERSION=$$version; \
	test -r $(FTPDIR)/$(PACKAGE).tar.Z || exit 0; \
	    cd $(FTPDIR) || exit 1;

$(FTPDIR)/$(PACKAGE)-$(VERSION).tar.Z:	$(PACKAGE)-$(VERSION).tar.Z
	rm -f $@
	cp $(PACKAGE)-$(VERSION).tar.Z $@ 
	chmod u+rw,g+rw,o=r $@

# Make a compressed, tar(1) file of the binary distribution in the 
# appropriate FTP directory.
#
binftp:		FORCE
	version=`cat VERSION`; \
	$(MAKE) $(MFLAGS) $(FTPBINDIR)/$(PACKAGE)-$$version.tar.Z \
	    VERSION=$$version; \
	$(MAKE) $(MFLAGS) $(FTPBINDIR)/$(PACKAGE)-$$version.env \
	    VERSION=$$version
ftpbin:		binftp

$(FTPBINDIR)/$(PACKAGE)-$(VERSION).tar.Z:
	rm -f $@
	id=$(PACKAGE)-$(VERSION) \
	&& rm -f $$id \
	&& ln -s $(prefix) $$id \
	&& tar $(TARFLAGS) - README $$id/bin $$id/include \
	    $$id/lib $$id/man | compress > $@ \
	&& rm $$id
	chmod u+rw,g+rw,o=r $@
	test -r $(FTPBINDIR)/$(PACKAGE).tar.Z || exit 0; \
	    cd $(FTPBINDIR) || exit 1;

$(FTPBINDIR)/$(PACKAGE)-$(VERSION).env:
	cp macros.make $@


include rules.make
