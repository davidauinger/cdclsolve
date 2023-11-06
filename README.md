This project provides a SAT solver to solve a Boolean formula in Conjunctive Normal Form (CNF) with Boolean Constraint Propagation (BCP) and Conflict Driven Clause Learning (CDCL).

# Compilation

A `CMakeLists.txt` file is provided with the project. To compile with cmake, run the following:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
An executable file `cdclsolve` should then be in the build directory.

# Installation

The `CMakeLists.txt` file supports installation with `make install`. If a custom installation path is desired, set the variable `CMAKE_INSTALL_PREFIX` when running cmake. This will install the project in the home directory:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~ ..
make
make install
```

# Dependencies

The project depends on Boost.Program_options.

# Usage

The input CNF has to be provided in [QDIMACS standard](https://www.qbflib.org/qdimacs.html). By default, the program reads the input CNF from stdin and writes the solution  (as defined by the QDIMACS standard output format) to stdout. Of course, these can be redirected to files. Furthermore, the decision heuristic must be set with the mandatory command line option `--decision`. A typical call might look like:
```
./cdclsolve --decision vsids < input > output
```

# Command line options

The complete list of the allowed command line options (with their short forms) and arguments is printed by invoking the program with the `--help` option. Following is a description of common usages of the options.

## Input and output

Instead of reading from stdin and writing to stdout, file names can also be specified with the `--input` and `--output` options, respectively.

## Decision heuristics

The decision heuristic must be chosen with the option `--decision`. Available decision heuristics are `basic`, `jeroslovwang`, `dlis`, and `vsids`.

## Measuring times

The times the solver spends for certain tasks can be measured with the option `--measure`. This will print a small table with two columns and four rows to stderr at the end. The four rows represent the times for the following tasks (from top to bottom): Boolean constraint propagation, conflict resolution, decision, total. The first row is the absolute time in seconds, the second row the relative time.
