#######################################################################
# Makefile
#   Build libconvd
#
# Author: 350137278@qq.com
#
# Update: 2021-06-22
#
# Show all predefinitions of gcc:
#
#   https://blog.csdn.net/10km/article/details/49023471
#
#   $ gcc -posix -E -dM - < /dev/null
#
#######################################################################
# Linux, CYGWIN_NT, MSYS_NT, ...
shuname="$(shell uname)"
OSARCH=$(shell echo $(shuname)|awk -F '-' '{ print $$1 }')

# debug | release (default)
RELEASE = 1
BUILDCFG = release

# 32 | 64 (default)
BITS = 64

# Is MINGW(1) or not(0)
MINGW_FLAG = 0

###########################################################
# Compiler Specific Configuration

CC = gcc

# for gcc-8+
# -Wno-unused-const-variable
CFLAGS += -std=gnu99 -D_GNU_SOURCE -fPIC -Wall -Wno-unused-function -Wno-unused-variable
#......

LDFLAGS += -liconv
#......


###########################################################
# Architecture Configuration

ifeq ($(RELEASE), 0)
	# debug: make RELEASE=0
	CFLAGS += -D_DEBUG -g
	BUILDCFG = debug
else
	# release: make RELEASE=1
	CFLAGS += -DNDEBUG -O3
	BUILDCFG = release
endif

ifeq ($(BITS), 32)
	# 32bits: make BITS=32
	CFLAGS += -m32
	LDFLAGS += -m32
else
	ifeq ($(BITS), 64)
		# 64bits: make BITS=64
		CFLAGS += -m64
		LDFLAGS += -m64
	endif
endif


ifeq ($(OSARCH), MSYS_NT)
	MINGW_FLAG = 1
	ifeq ($(BITS), 64)
		CFLAGS += -D__MINGW64__
	else
		CFLAGS += -D__MINGW32__
	endif
else ifeq ($(OSARCH), MINGW64_NT)
	MINGW_FLAG = 1
	ifeq ($(BITS), 64)
		CFLAGS += -D__MINGW64__
	else
		CFLAGS += -D__MINGW32__
	endif
else ifeq ($(OSARCH), MINGW32_NT)
	MINGW_FLAG = 1
	ifeq ($(BITS), 64)
		CFLAGS += -D__MINGW64__
	else
		CFLAGS += -D__MINGW32__
	endif
endif


ifeq ($(OSARCH), CYGWIN_NT)
	ifeq ($(BITS), 64)
		CFLAGS += -D__CYGWIN64__ -D__CYGWIN__
	else
		CFLAGS += -D__CYGWIN32__ -D__CYGWIN__
	endif
endif


###########################################################
# Project Specific Configuration
PREFIX = .
DISTROOT = $(PREFIX)/dist
APPS_DISTROOT = $(DISTROOT)/apps

# Given dirs for all source (*.c) files
SRC_DIR = $(PREFIX)/src
LIBS_DIR = $(PREFIX)/libs
COMMON_DIR = $(SRC_DIR)/common
APPS_DIR = $(SRC_DIR)/apps


#----------------------------------------------------------
# convd

CONVD_DIR = $(SRC_DIR)/convd
CONVD_VERSION_FILE = $(CONVD_DIR)/VERSION
CONVD_VERSION = $(shell cat $(CONVD_VERSION_FILE))

CONVD_STATIC_LIB = libconvd.a
CONVD_DYNAMIC_LIB = libconvd.so.$(CONVD_VERSION)

CONVD_DISTROOT = $(DISTROOT)/libconvd-$(CONVD_VERSION)
CONVD_DIST_LIBDIR=$(CONVD_DISTROOT)/lib/$(OSARCH)/$(BITS)/$(BUILDCFG)
#----------------------------------------------------------


# add other projects here:
#...


# Set all dirs for C source: './src/a ./src/b'
ALLCDIRS += $(SRCDIR) \
	$(COMMON_DIR) \
	$(CONVD_DIR)
#...


# Get pathfiles for C source files: './src/a/1.c ./src/b/2.c'
CSRCS := $(foreach cdir, $(ALLCDIRS), $(wildcard $(cdir)/*.c))

# Get names of object files: '1.o 2.o'
COBJS = $(patsubst %.c, %.o, $(notdir $(CSRCS)))


# Given dirs for all header (*.h) files
INCDIRS += -I$(PREFIX) \
	-I$(SRC_DIR) \
	-I$(COMMON_DIR) \
	-I$(CONVD_DIR)
#...


ifeq ($(MINGW_FLAG), 1)
	MINGW_CSRCS = $(COMMON_DIR)/win32/syslog-client.c
	MINGW_LINKS = -lws2_32
else
	MINGW_CSRCS =
	MINGW_LINKS = -lrt
endif

MINGW_COBJS = $(patsubst %.c, %.o, $(notdir $(MINGW_CSRCS)))

###########################################################
# Build Target Configuration
.PHONY: all apps clean cleanall dist


all: $(CONVD_DYNAMIC_LIB).$(OSARCH) $(CONVD_STATIC_LIB).$(OSARCH)

#...


#----------------------------------------------------------
# http://www.gnu.org/software/make/manual/make.html#Eval-Function

define COBJS_template =
$(basename $(notdir $(1))).o: $(1)
	$(CC) $(CFLAGS) -c $(1) $(INCDIRS) -o $(basename $(notdir $(1))).o
endef
#----------------------------------------------------------


$(foreach src,$(CSRCS),$(eval $(call COBJS_template,$(src))))

$(foreach src,$(MINGW_CSRCS),$(eval $(call COBJS_template,$(src))))


help:
	@echo
	@echo "Build all libs and apps as the following"
	@echo
	@echo "Build 64 bits release (default):"
	@echo "    $$ make clean && make"
	@echo
	@echo "Build 32 bits debug:"
	@echo "    $$ make clean && make RELEASE=0 BITS=32"
	@echo
	@echo "Dist target into default path:"
	@echo "    $$ make clean && make dist"
	@echo
	@echo "Dist target into given path:"
	@echo "    $$ make CONVD_DISTROOT=/path/to/YourInstallDir dist"
	@echo
	@echo "Build apps with all libs:"
	@echo "    $$ make clean && make apps"
	@echo
	@echo "Show make options:"
	@echo "    $$ make help"


#----------------------------------------------------------
$(CONVD_STATIC_LIB).$(OSARCH): $(COBJS) $(MINGW_COBJS)
	rm -f $@
	rm -f $(CONVD_STATIC_LIB)
	ar cr $@ $^
	ln -s $@ $(CONVD_STATIC_LIB)

$(CONVD_DYNAMIC_LIB).$(OSARCH): $(COBJS) $(MINGW_COBJS)
	$(CC) $(CFLAGS) -shared \
		-Wl,--soname=$(CONVD_DYNAMIC_LIB) \
		-Wl,--rpath='$(PREFIX):$(PREFIX)/lib:$(PREFIX)/../lib:$(PREFIX)/../libs/lib' \
		-o $@ \
		$^ \
		$(LDFLAGS) \
		$(MINGW_LINKS)
	ln -s $@ $(CONVD_DYNAMIC_LIB)
#----------------------------------------------------------


apps: dist codeconv.exe.$(OSARCH)


# -lrt for Linux
codeconv.exe.$(OSARCH): $(APPS_DIR)/codeconv/codeconv.c
	@echo Building codeconv.exe.$(OSARCH)
	$(CC) $(CFLAGS) $< $(INCDIRS) \
	-o $@ \
	$(CONVD_STATIC_LIB) \
	$(LDFLAGS) \
	$(MINGW_LINKS)
	ln -sf $@ codeconv


dist: all
	@mkdir -p $(CONVD_DISTROOT)/include/common
	@mkdir -p $(CONVD_DISTROOT)/include/convd
	@mkdir -p $(CONVD_DIST_LIBDIR)
	@cp $(COMMON_DIR)/unitypes.h $(CONVD_DISTROOT)/include/common/
	@cp $(CONVD_DIR)/convd_api.h $(CONVD_DISTROOT)/include/convd/
	@cp $(CONVD_DIR)/convd_def.h $(CONVD_DISTROOT)/include/convd/
	@cp $(PREFIX)/$(CONVD_STATIC_LIB).$(OSARCH) $(CONVD_DIST_LIBDIR)/
	@cp $(PREFIX)/$(CONVD_DYNAMIC_LIB).$(OSARCH) $(CONVD_DIST_LIBDIR)/
	@cd $(CONVD_DIST_LIBDIR)/ && ln -sf $(PREFIX)/$(CONVD_STATIC_LIB).$(OSARCH) $(CONVD_STATIC_LIB)
	@cd $(CONVD_DIST_LIBDIR)/ && ln -sf $(PREFIX)/$(CONVD_DYNAMIC_LIB).$(OSARCH) $(CONVD_DYNAMIC_LIB)
	@cd $(CONVD_DIST_LIBDIR)/ && ln -sf $(CONVD_DYNAMIC_LIB) libconvd.so


clean:
	-rm -f *.stackdump
	-rm -f $(COBJS) $(MINGW_COBJS)
	-rm -f $(CONVD_STATIC_LIB)
	-rm -f $(CONVD_DYNAMIC_LIB)
	-rm -f $(CONVD_STATIC_LIB).$(OSARCH)
	-rm -f $(CONVD_DYNAMIC_LIB).$(OSARCH)
	-rm -rf ./msvc/libconvd/build
	-rm -rf ./msvc/libconvd/target
	-rm -rf ./msvc/libconvd_dll/build
	-rm -rf ./msvc/libconvd_dll/target
	-rm -rf ./msvc/codeconv/build
	-rm -rf ./msvc/codeconv/target
	-rm -f codeconv.exe.$(OSARCH)
	-rm -f codeconv
	-rm -f ./msvc/*.VC.db


cleanall: clean
	-rm -rf $(DISTROOT)