# set the binaries that have to be built

#SRCS := $(wildcard apps/mnist/*.cpp)
SRCS := ${SRCS} $(wildcard tests/*.cpp) 

TARGETS := $(TARGETS) $(SRCS:.cpp=) 

# set the build configuration set 
BUILD := release
#BUILD := debug
BIT := 64
#BIT := 32

# set bin and build dirs
BUILDDIR := .build_$(BUILD)$(BIT)
BINDIR := $(BUILD)$(BIT)

# include directories
INCLUDEDIRS := \
	src
# library directories
LIBDIRS :=

# set which libraries are used by which executable
LDLIBS = $(addprefix -l, $(LIBS) $(LIBS_$(notdir $*)))
LIBS =
#LIBS = pthread

# set some flags and compiler/linker specific commands
CXXFLAGS = -m$(BIT) -pipe -D STD=std -Wall $(CXXFLAGS_$(BUILD)) $(addprefix -I, $(INCLUDEDIRS))
CXXFLAGS_debug := -ggdb -std=c++1z
CXXFLAGS_release := -std=c++1z -O3 -DNDEBUG
LDFLAGS = -m$(BIT) -pipe -Wall $(LDFLAGS_$(BUILD)) $(addprefix -L, $(LIBDIRS))
LDFLAGS_debug := -ggdb
LDFLAGS_release := -O3

# we set specific compilers for specific tools
CXX = $(if $(CXX_$(notdir $*)), $(CXX_$(notdir $*)), g++)

include ./make/generic.mk
