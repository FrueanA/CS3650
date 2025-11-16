# This is a Makefile, containing instructions for the Make program. Make is a
# _build tool_ which helps managing the set up and compilation of programs,
# especially as their size and complexity increases.
#
# Read the tutorial at https://makefiletutorial.com/
# 

## SKIP LOOKING AT THIS PART ##
NO_PIE := $(shell echo "int main() { return 0; }" | gcc -no-pie -xc - -o /dev/null 2> /dev/null; echo $$?)

ifneq ($(NO_PIE), 0)
CFLAGS=
else
CFLAGS=-no-pie
endif
## SKIP UNTIL HERE ##



# A Makefile is a collection of targets, their dependencies and recipes.  The
# skeleton is roughly like this:
#
# target: dependency1 dependency2 ...
# <tab>	recipe
#
# The target says what we are trying to make; the dependencies what needs exist
# or what other target needs to be completed before we can attempt making the
# current one; and the recipe describes how to make the target as a collection
# of command lines.
#
# Tagets can be actual filenames, then they mean: this is how you create this
# file. They can also be "abstract" - just some appropriate name for an action.
#
# Dependencies can be files. One of the strengths of make is, that it will
# re-make a target only if any of the dependencies doesn't exist, or if it was
# updated since the last time the target was made.

# "all" is the default target which will be attempted if we call make without any 
# arguments. Here we say, to make all, we need to make a calculator.
all: calculator

# To make a "calculator" (which is the name of an executable file) we state
# that it depends on the assembly file "calculator.s". Now make will only
# re-compile calculator if "calculator.s" changes. To make the calculator, we
# run gcc with "calculator.s" as the source file and name the executable
# "calculator".
calculator: calculator.s 
	gcc $(CFLAGS) calculator.s -o calculator

# The (non-file) clean target will remove the executable and any intermediate
# binary files. Run this before adding files to your repo
clean:
	rm -f calculator
	rm -f *.o

# We tell Make that "clean" is not a file target and even if a file called 
# clean already exists, run the recipe.
.PHONY: clean
