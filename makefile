
all: default-build

EXTRA_CFLAGS   += -std=c99
EXTRA_CXXFLAGS += -std=c++11

MAKEFILEDIR := $(shell rdlib-config --makefiles)

include $(MAKEFILEDIR)/makefile.init

EXTRA_CFLAGS   += $(call pkgcflags,rdlib-0.1)
EXTRA_CXXFLAGS += $(call pkgcxxflags,rdlib-0.1)
EXTRA_CXXFLAGS += $(shell wx-config --cppflags) -Wno-ignored-qualifiers -Wno-cast-function-type -Wno-deprecated-copy
EXTRA_LIBS	   += $(call pkglibs,rdlib-0.1)
EXTRA_LIBS	   += $(shell wx-config --libs)

INITIAL_COMMON_FLAGS := $(LOCAL_COMMON_FLAGS)

APPLICATION	       := redirector
LOCAL_COMMON_FLAGS := $(INITIAL_COMMON_FLAGS) -I$(APPLICATION)
OBJECTS		       := $(APPLICATION:%=%.o) redirectorwindow.o
include $(MAKEFILEDIR)/makefile.app

APPLICATION        := viewer
LOCAL_COMMON_FLAGS := $(INITIAL_COMMON_FLAGS) -I$(APPLICATION)
OBJECTS		       := $(APPLICATION:%=%.o) $(APPLICATION:%=%app.o)
include $(MAKEFILEDIR)/makefile.app

include $(MAKEFILEDIR)/makefile.post
