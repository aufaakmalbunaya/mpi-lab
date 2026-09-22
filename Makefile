# MPI Lab — build all exercises from src/ into bin/
#
# Usage:
#   make            build all programs
#   make hello      build one program
#   make clean      remove compiled binaries
#
# Run a program, e.g.:
#   mpirun -np 4 bin/hello

CC       := mpicc
CFLAGS   := -std=c11 -O2 -Wall -Wextra
SRCDIR   := src
BINDIR   := bin

PROGRAMS := hello request_reply ring broadcast scatter_sum allreduce \
            nonblocking_ring pi_mpi

TARGETS  := $(addprefix $(BINDIR)/,$(PROGRAMS))

.PHONY: all clean list

all: $(TARGETS)

$(BINDIR)/%: $(SRCDIR)/%.c | $(BINDIR)
	$(CC) $(CFLAGS) $< -o $@

$(BINDIR):
	mkdir -p $(BINDIR)

# Convenience: `make hello` builds bin/hello
$(PROGRAMS): %: $(BINDIR)/%

clean:
	rm -f $(TARGETS)

list:
	@echo "Programs: $(PROGRAMS)"
