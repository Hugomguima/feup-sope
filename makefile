# ---- C compiler
CC =gcc

# ---- Directories
# Source files
SDIR =./src
# Header files
IDIR =./include
# Library files
LDIR =./lib
# Object files
ODIR =./obj
# Executable files
BDIR =./bin

# Flags
CFLAGS =-Wall -Wextra -Werror -Wpedantic -pedantic
IFLAGS =-I$(IDIR)
LFLAGS =-L$(LDIR)

# Dependencies
DEPS =$(ODIR)/utils.o $(ODIR)/parse.o $(ODIR)/log.o $(ODIR)/cleanup.o
MAIN =main.o

# Executable
TARGET =simpledu

.PHONY: all clean

all: $(BDIR)/$(TARGET)

# Create object files
$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -c $(IFLAGS) $< -o $@

# Create library
makelib: makefolders $(LDIR)/lib.a

$(LDIR)/lib.a: $(DEPS)
	rm -f $@
	ar rvs $@ $(DEPS)

$(BDIR)/$(TARGET): makelib $(ODIR)/$(MAIN)
	$(CC) $(CFLAGS) -o $@ $(word 2, $^) $(DEPS)

makefolders:
	mkdir -p $(LDIR)
	mkdir -p $(ODIR)
	mkdir -p $(BDIR)

clean:
	rm -rf $(LDIR)
	rm -rf $(ODIR)
	rm -rf $(BDIR)
