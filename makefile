
all: default-build

MAKEFILEDIR = /usr/local/share/rdlib-0.1/makefiles

include $(MAKEFILEDIR)/makefile.init

EXTRA_CFLAGS += $(shell pkg-config --cflags rdlib-0.1)
EXTRA_CFLAGS += $(shell wx-config --cppflags)
EXTRA_LIBS   += $(shell pkg-config --libs rdlib-0.1)
EXTRA_LIBS   += $(shell wx-config --libs)

LOCAL_CFLAGS := -Iredirector

APPLICATION := redirector
OBJECTS     := $(APPLICATION:%=%.o) redirectorwindow.o
include $(MAKEFILEDIR)/makefile.app

APPLICATION := viewer
OBJECTS     := $(APPLICATION:%=%.o) viewerapp.o
include $(MAKEFILEDIR)/makefile.app

include $(MAKEFILEDIR)/makefile.post
