#
# Makefile for compiler and linker feedback.
#

obj-$(CONFIG_FEEDBACK_COLLECT) += cachepack.o asm.o libgcov-kernel.o

# Don't build the cachepack code itself to collect feedback.
arch/tile/feedback/cachepack.o: KBUILD_CFLAGS := \
  $(filter-out -ffeedback-generate%, $(KBUILD_CFLAGS))

# Ditto for instrumentation code.
arch/tile/feedback/libgcov-kernel.o: KBUILD_CFLAGS := \
  $(filter-out -ffeedback-generate%, $(KBUILD_CFLAGS))
