#
# GNSS World Nanoapp Makefile
#

# Environment Checks ###########################################################

ifeq ($(CHRE_PREFIX),)
ifneq ($(ANDROID_BUILD_TOP),)
CHRE_PREFIX = $(ANDROID_BUILD_TOP)/system/chre
else
$(error "You must run 'lunch' to setup ANDROID_BUILD_TOP, or explicitly define \
         the CHRE_PREFIX environment variable to point to the CHRE root \
	 directory.")
endif
endif

# Nanoapp Configuration ########################################################

NANOAPP_NAME = gnss_world
NANOAPP_ID = 0x0123456789000005
NANOAPP_VERSION = 0x00000001

NANOAPP_NAME_STRING = \"GNSS\ World\"

# Common Compiler Flags ########################################################

# Include paths.
COMMON_CFLAGS += -I.
COMMON_CFLAGS += -I$(CHRE_PREFIX)/util/include

# Defines
COMMON_CFLAGS += -DNANOAPP_MINIMUM_LOG_LEVEL=CHRE_LOG_LEVEL_DEBUG
COMMON_CFLAGS += -DCHRE_ASSERTIONS_ENABLED

# Common Source Files ##########################################################

COMMON_SRCS += gnss_world.cc

# Permission declarations ######################################################

CHRE_NANOAPP_USES_GNSS = true

# Makefile Includes ############################################################

include $(CHRE_PREFIX)/build/nanoapp/app.mk
