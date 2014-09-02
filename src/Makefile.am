bin_PROGRAMS =									\
	redirector									\
	viewer

redirector_LDADD =								\
	$(RDLIB_LIBS)								\
	$(WX_LIBS)

redirector_CPPFLAGS =							\
	$(RDLIB_CFLAGS)								\
	$(RDUTILS_CFLAGS)							\
	$(WX_CPPFLAGS)

redirector_SOURCES = redirector.cpp redirectorwindow.cpp

viewer_LDADD =									\
	$(RDLIB_LIBS)								\
	$(WX_LIBS)

viewer_CPPFLAGS =								\
	$(RDLIB_CFLAGS)								\
	$(RDUTILS_CFLAGS)							\
	$(WX_CPPFLAGS)

viewer_SOURCES = viewer.cpp viewerapp.cpp

DISTCLEANFILES = rdwxutils-stdint.h
