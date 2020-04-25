# ---- C compiler
CC = gcc

# ---- Directories
# --- Client
# Source files
USDIR = ./client/src
# Header files
UIDIR = ./client/include
# Library files
ULDIR = ./client/lib
# Object files
UODIR = ./client/obj

# --- Server
# Source files
QSDIR = ./server/src
# Header files
QIDIR = ./server/include
# Library files
QLDIR = ./server/lib
# Object files
QODIR = ./server/obj

# --- Shared Files
SIDIR = ./shared/include
SSDIR = ./shared/src

# Executable files
BDIR = ./bin

# ---- Flags
CFLAGS = -Wall -pthread
# --- Client
UIFLAGS = -I$(UIDIR)
ULFLAGS = -L$(ULDIR)
# --- Server
QIFLAGS = -I$(QIDIR)
QLFLAGS = -L$(QLDIR)


# Dependencies
# --- Client Dependencies
UDEPS = $(UODIR)/utils.o $(UODIR)/parse.o
# --- Server Dependencies
QDEPS = $(QODIR)/utils.o $(QODIR)/parse.o
MAIN = main.o

# Executable
UTARGET = Un
QTARGET = Qn

.PHONY: all clean

all: $(BDIR)/$(UTARGET) $(BDIR)/$(QTARGET)

# Create object files
# --- Shared
$(UODIR)/%.o: $(SSDIR)/%.c
	$(CC) $(CFLAGS) -c $(SIFLAGS) $< -o $@
$(QODIR)/%.o: $(SSDIR)/%.c
	$(CC) $(CFLAGS) -c $(SIFLAGS) $< -o $@

# --- Client
$(UODIR)/%.o: $(USDIR)/%.c
	$(CC) $(CFLAGS) -c $(UIFLAGS) $< -o $@

# --- Server
$(QODIR)/%.o: $(QSDIR)/%.c
	$(CC) $(CFLAGS) -c $(QIFLAGS) $< -o $@

# Create library
makelib: makefolders $(ULDIR)/lib.a $(QLDIR)/lib.a

$(ULDIR)/lib.a: $(UDEPS)
	rm -f $@
	ar rvs $@ $(UDEPS)

$(QLDIR)/lib.a: $(QDEPS)
	rm -f $@
	ar rvs $@ $(QDEPS)


$(BDIR)/$(UTARGET): makelib $(UODIR)/$(MAIN)
	$(CC) $(CFLAGS) -o $@ $(word 2, $^) $(UDEPS)

$(BDIR)/$(QTARGET): makelib $(QODIR)/$(MAIN)
	$(CC) $(CFLAGS) -o $@ $(word 2, $^) $(QDEPS)

makefolders:
	mkdir -p $(ULDIR)
	mkdir -p $(UODIR)
	mkdir -p $(QLDIR)
	mkdir -p $(QODIR)
	mkdir -p $(BDIR)

clean:
	rm -rf $(ULDIR)
	rm -rf $(UODIR)
	rm -rf $(QLDIR)
	rm -rf $(QODIR)
	rm -rf $(BDIR)
