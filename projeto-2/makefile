# ---- C compiler
CC = gcc

# ---- Directories
# --- Shared
# Source files
SSDIR = ./shared/src
# Header files
SIDIR = ./shared/include
# Library files
SLDIR = ./shared/lib
# Object files
SODIR = ./shared/obj

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


# Executable files
BDIR = ./bin

# ---- Flags
# --- Shared
SIFLAGS = -I$(SIDIR)
SLFLAGS = -L$(SLDIR)
SCFLAGS = -Wall -pthread
# --- Client
UIFLAGS = -I$(UIDIR) $(SIFLAGS)
ULFLAGS = -L$(ULDIR) $(SLFLAGS)
UCFLAGS = -Wall -pthread
# --- Server
QIFLAGS = -I$(QIDIR) $(SIFLAGS)
QLFLAGS = -L$(QLDIR) $(SLFLAGS)
QCFLAGS = -Wall -pthread


# Dependencies
# --- Shared Dependencies
SDEPS = $(SODIR)/utils.o $(SODIR)/protocol.o $(SODIR)/sync.o $(SODIR)/log.o $(SODIR)/error.o
# --- Client Dependencies
UDEPS = $(SDEPS) $(UODIR)/parse.o
UMAIN = main.o
# --- Server Dependencies
QDEPS = $(SDEPS) $(QODIR)/parse.o
QMAIN = main.o

# Executable
UTARGET = U1
QTARGET = Q1

.PHONY: all clean client server shared

all: client server

client: shared $(BDIR)/$(UTARGET)

server: shared $(BDIR)/$(QTARGET)

shared: makesharedlib

# Create object files
# --- Shared
$(SODIR)/%.o: $(SSDIR)/%.c
	$(CC) $(SCFLAGS) -c $(SIFLAGS) $< -o $@

# --- Client
$(UODIR)/%.o: $(USDIR)/%.c
	$(CC) $(CFLAGS) -c $(UIFLAGS) $< -o $@

# --- Server
$(QODIR)/%.o: $(QSDIR)/%.c
	$(CC) $(CFLAGS) -c $(QIFLAGS) $< -o $@

# Create library
makelib: makefolders makesharedlib makeclientlib makeserverlib

makesharedlib: makesharedfolders $(SLDIR)/lib.a

makeclientlib: makesharedlib makeclientfolders $(ULDIR)/lib.a

makeserverlib: makesharedlib makeserverfolders $(QLDIR)/lib.a

$(SLDIR)/lib.a: $(SDEPS)
	rm -rf $@
	ar rvs $@ $<

$(ULDIR)/lib.a: $(UDEPS)
	rm -f $@
	ar rvs $@ $<

$(QLDIR)/lib.a: $(QDEPS)
	rm -f $@
	ar rvs $@ $<


$(BDIR)/$(UTARGET): makeclientlib $(UODIR)/$(UMAIN)
	$(CC) $(UCFLAGS) -o $@ $(word 2, $^) $(UDEPS)
	ln -sf $@ $(UTARGET)

$(BDIR)/$(QTARGET): makeserverlib $(QODIR)/$(QMAIN)
	$(CC) $(QCFLAGS) -o $@ $(word 2, $^) $(QDEPS)
	ln -sf $@ $(QTARGET)

makefolders: makesharedfolders makeclientfolders makeserverfolders

makesharedfolders:
	mkdir -p $(SLDIR)
	mkdir -p $(SODIR)

makeclientfolders:
	mkdir -p $(ULDIR)
	mkdir -p $(UODIR)
	mkdir -p $(BDIR)

makeserverfolders:
	mkdir -p $(QLDIR)
	mkdir -p $(QODIR)
	mkdir -p $(BDIR)

clean: cleanshared
	rm -rf $(BDIR)

cleanshared: cleanclient cleanserver
	rm -rf $(SLDIR)
	rm -rf $(SODIR)
cleanclient:
	rm -f  $(UTARGET)
	rm -rf $(ULDIR)
	rm -rf $(UODIR)
cleanserver:
	rm -f  $(QTARGET)
	rm -rf $(QLDIR)
	rm -rf $(QODIR)
