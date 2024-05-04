##############################################################################
# Project Configuration
#

PROJECT := lily-nz
PROJECT_TYPE := osc

##############################################################################
# Sources
#

EURORACKDIR = ../eurorack
BRAIDSDIR   = $(EURORACKDIR)/braids
STMLIBDIR   = $(EURORACKDIR)/stmlib
SRCDIR      = ../common

# C sources
UCSRC = header.c

# C++ sources
UCXXSRC = $(SRCDIR)/unit.cc $(SRCDIR)/digital_oscillator.cc $(SRCDIR)/resources.cc $(BRAIDSDIR)/quantizer.cc $(SRCDIR)/macro_oscillator.cc $(STMLIBDIR)/utils/random.cc

# List ASM source files here
ASMSRC = 

ASMXSRC = 

##############################################################################
# Include Paths
#

UINCDIR  = $(EURORACKDIR) $(STMLIBDIR) $(STMLIBDIR)/third_party/STM $(STMLIBDIR)/third_party/STM/STM32F10x_StdPeriph_Driver/inc/ $(STMLIBDIR)/third_party/STM/CMSIS/CM3_f10x

##############################################################################
# Library Paths
#

ULIBDIR = 

##############################################################################
# Libraries
#

ULIBS  = -lm
ULIBS += -lc

##############################################################################
# Macros
#

UDEFS = -DLILYNZ

