RANDOM := $(shell bash -c 'which clang++-10')

ifneq (, $(shell which clang++-10))
  COMPILER  =  clang++-10
  LINKER = wasm-ld-10
else
  COMPILER  =  clang++-9
  LINKER = wasm-ld-9
endif

#COMPILER  =  clang++

CFLAGS := -MMD -MP -Wall -Wextra -Winit-self -Wno-missing-field-initializers -Werror -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-private-field#-msse 
CFLAGS := $(CFLAGS) -std=c++11 --target=wasm32-wasi --sysroot wasi-sysroot -O3
#CFLAGS := $(CFLAGS) -g -DNDEBUG


#LDFLAGS :=   --export=__heap_base --export=calling --allow-undefined --no-entry 
#LDFLAGS := --export-all  --export=__heap_base --export=calling --allow-undefined --no-entry 
LDFLAGS := --strip-all  --export=__heap_base --export=calling --allow-undefined --no-entry 
LDFLAGS := --lto-O3 -O3 --import-memory -o result.wasm $(LDFLAGS)
#LDFLAGS := $(LDFLAGS) --allow-undefined-file=undefined.syms


SYSLibDIR   = wasi-sysroot/lib/wasm32-wasi
LIBS      = $(SYSLibDIR)/libc.a $(SYSLibDIR)/libc++.a $(SYSLibDIR)/libc++abi.a

INCLUDE   = -I./include -I../src/
TARGET    = result.wasm
SRCDIR    = ./
ifeq "$(strip $(SRCDIR))" ""
  SRCDIR  = .
endif

SOURCES   = $(wildcard $(SRCDIR)/wasm.cpp) $(wildcard ../src/codenodes/*.cpp)
OBJDIR    = ./
ifeq "$(strip $(OBJDIR))" ""
  OBJDIR  = .
endif
OBJECTS   = $(addprefix $(OBJDIR)/, $(notdir $(SOURCES:.cpp=.o)))
DEPENDS   = $(OBJECTS:.o=.d)



$(TARGET): $(OBJECTS) $(LIBS)
	$(LINKER) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	-mkdir -p $(OBJDIR)
	$(COMPILER) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(OBJDIR)/%.o: ../src/codenodes/%.cpp
	-mkdir -p $(OBJDIR)
	$(COMPILER) $(CFLAGS) $(INCLUDE) -o $@ -c $<

all: clean $(TARGET)
	-rm -f $(OBJECTS) $(DEPENDS) $(TARGET)
clean: $(TARGET)
	-rm -f $(OBJECTS) $(DEPENDS) $(TARGET)


-include $(DEPENDS)
