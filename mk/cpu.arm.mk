LDFILE =	$(ARCHDIR)/arm/system.ld
SRC +=		$(ARCHDIR)/arm/init.cpp \
			$(ARCHDIR)/arm/stdcalls.cpp \
			$(ARCHDIR)/arm/supcxx.cpp \
			$(ARCHDIR)/arm/syscallstable.cpp \
			$(ARCHDIR)/arm/criticalsection.cpp \
			$(ARCHDIR)/arm/semihost.cpp
