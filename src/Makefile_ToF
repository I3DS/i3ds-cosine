.PHONY       : all clean depend

NAME         := FirstSample

SRCS         := FirstSample.cpp

OBJS         := $(SRCS:.cpp=.o)

# Installation directory for GenICam and GenTL
GENTL_ROOT   ?= /opt/BaslerToF

GENAPI_POSTFIX  := gcc_v3_0_Basler_pylon_v5_0

# Where to find the GenApi libraries
LIBDIR := $(shell case `arch` in \
                            (x86_64) echo "$(GENTL_ROOT)/lib64" ;; \
                            (*)      echo "$(GENTL_ROOT)/lib" ;; \
                        esac)
                        
# Build tools and flags
CXX		    ?= g++
LD		    := $(CXX)
INCLUDES   	?= -I$(GENTL_ROOT)/include
CPPFLAGS   	:= $(INCLUDES)
CXXFLAGS   	:= -std=c++11 -Wall -Wno-unknown-pragmas #e.g., CXXFLAGS=-g -O0 for debugging
LIBS       	:= -L$(LIBDIR) -lGenApi_$(GENAPI_POSTFIX) -lGCBase_$(GENAPI_POSTFIX) -ldl
# !! The following linker options are required for finding the GenTL producer .cti file and its 
# dependencies!
LDFLAGS    	:= -Wl,--enable-new-dtags -Wl,-rpath,$(LIBDIR) -Wl,-rpath,$(LIBDIR)/gentlproducer/gtl 
# !! This linker option is always required when using GenApi. Without the -Wl,-E option 
# runtime type information is not handled properly. Correct runtime type information is particularly 
# important for handling exceptions thrown by the GenApi library!
LDFLAGS		+= -Wl,-E

# Rules for building
all: $(NAME) 

$(NAME): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(NAME) 

depend: $(SRCS)
		makedepend -Y $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
