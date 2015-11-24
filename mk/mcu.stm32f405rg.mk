CPPFLAGS += \
	-DMCU_CMSIS_HEADER=vendor/cmsis/stm32f405xx.h \
	-DMCU_MEMORY_MAP=stm32/mmap.stm32f40x.i \
	-DMCU_NUM_IRQn=82
TARGET_CPU = cortex-m4
