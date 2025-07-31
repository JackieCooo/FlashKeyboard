include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := $(notdir $(shell pwd))

LOCAL_SRCS := $(wildcard *.c)
LOCAL_SRCS_NEED_SORT := y

LOCAL_INCLUDE := \
	-I $(LITEOSTOPDIR)/kernel/extended/include

LOCAL_FLAGS := $(LOCAL_INCLUDE) $(LITEOS_GCOV_OPTS) $(LITEOS_NON_SECURE_LOCAL_OPTS)

include $(MODULE)
