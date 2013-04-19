CONTIKI_PROJECT = gateway
all: $(CONTIKI_PROJECT)

ZENZERO = ../..

TARGETDIRS += $(ZENZERO)/platform

# needed for linking for the sdcc system under cygwin only when large model is used
# LDFLAGS += -L /usr/local/share/sdcc/lib/large

CONTIKI_NO_NET = 1

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

CONTIKI = $(ZENZERO)/contiki

# if you want ovveride how the platform will be built set the env PLATFORM, ie:
# export PLATFORM=zenziki
# where zenziki is the directory that contains the makefiles recipes
#PLATFORM ?= $(CONTIKI)
PLATFORM ?= $(ZENZERO)/apps

include $(PLATFORM)/Makefile.include

#include $(CONTIKI)/Makefile.include

#vprint:
#	@echo "CONTIKI_CPU_DIRS_CONCAT: $(CONTIKI_CPU_DIRS_CONCAT)"
#	@echo "CONTIKI_TARGET_DIRS_CONCAT: $(CONTIKI_TARGET_DIRS_CONCAT)"

