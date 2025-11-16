[![Open in Codespaces](https://classroom.github.com/assets/launch-codespace-2972f46106e565e64193e422d61a12cf1da4916b45550586e14ef0a7c637dd04.svg)](https://classroom.github.com/open-in-codespaces?assignment_repo_id=21596765)
# Assignment 6: Concurrent Sorting

This is the starter code for [Assignment 6](https://khoury-cs3650.github.io/a06.html).

The [Makefile](Makefile) contains the following targets:

- `make all` - compile `msort` and `tmsort`
- `make msort` and `make tmsort` - compile the individual programs
- `make diff-N` - compile and run a diff test, comparing the results of `msort` and `tmsort` on a random input. `N` needs to be replaced by a positive integer. E.g., `make diff-100`.
- `make clean` - perform a minimal clean-up of the source tree
- `make clean-temp` - perform a cleanup of temporary files created since the last run of this target
- `make valgrind` - run `valgrind` on both `msort` and `tmsort`. By default uses 1000 as the number of elements

Note: This Makefile asks `gcc` to convert warnings into errors to help draw your attention to them.
