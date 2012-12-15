# Compiler properties
CPPC = i386-pc-chickenos-g++
LD = i386-pc-chickenos-g++
#CFLAGS = -g -Wall
#LFLAGS = -g
CFLAGS =
LFLAGS =

# Output executable file name and output directory
PROG = XLSE
OUTDIR = ../

# Source files and object files to generate
SRCS = script.cpp factory.cpp interpreter.cpp lisp.cpp main.cpp parser.cpp primitives.cpp references.cpp environment.cpp
OBJS = script.o factory.o interpreter.o lisp.o main.o parser.o primitives.o references.o environment.o

# Compilation target:
compile: $(PROG)

# Cleaning target
clean:
	rm -f *.o

# Rule for linking executables
$(PROG): $(OBJS)
	$(LD) $(LFLAGS) $(OBJS) -o $(OUTDIR)$(PROG)

# Meta-rule for compiling C++ source files
%.o: %.cpp
	$(CPPC) $(CFLAGS) -c $<
