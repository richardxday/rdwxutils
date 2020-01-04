
all: default-build

MAKEFILEDIR := $(shell rdlib-config --makefiles)

include $(MAKEFILEDIR)/makefile.init

EXTRA_CFLAGS += $(shell pkg-config --cflags rdlib-0.1)
EXTRA_CFLAGS += $(shell wx-config --cppflags) -Wno-ignored-qualifiers -Wno-cast-function-type -Wno-deprecated-copy
EXTRA_LIBS	 += $(shell pkg-config --libs rdlib-0.1)
EXTRA_LIBS	 += $(shell wx-config --libs)

APPLICATION	 := redirector
LOCAL_CFLAGS += -I$(APPLICATION)
OBJECTS		 := $(APPLICATION:%=%.o) redirectorwindow.o
include $(MAKEFILEDIR)/makefile.app

APPLICATION := viewer
OBJECTS		:= $(APPLICATION:%=%.o) viewerapp.o
include $(MAKEFILEDIR)/makefile.app

include $(MAKEFILEDIR)/makefile.post
