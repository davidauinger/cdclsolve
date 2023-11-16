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

# Fuzz testing

The script `cnfuzzrep.sh` provides a simple way of fuzz testing the SAT solver. In addition to `cdclsolve`, this script requires `cnfuzz` ([available here](https://fmv.jku.at/cnfuzzdd/)) and `lingeling` ([available here](https://github.com/arminbiere/lingeling)).

## Usage

The script has to be called with the output directory as a single paramter. Several options are provided to adjust the behavior as desired. The complete command line syntax looks like this:
```
cnfuzzrep.sh [-j parallelJobs|-n] [-c count] [-t timeout] [-s seed] outputDir
```

The script implements a signal handler for the signal SIGTERM. Sending a SIGTERM signal will stop the script from generating further formulas and shut down until all the open jobs are completed.

Parallel jobs can be started either with `-j parallelJobs`, where `parallelJobs` is the number of parallel jobs to be executed, or `-n`, where as many parallel jobs are processing cores are available will be executed. If this option is not provided, only one formula will be processed at a time.

The number of formulas can be limited with `-c count`, where `count` is the maximum number of formulas to be processed. If this option is not provided, the script will continue processing formula until a failure is found, or until a SIGTERM signal is received.

A timeout for solving the formula can be specified with `-t timeout`, where `timeout` is directly passed as duration to the `timeout` command. If this option is not provided, no timeout is assumed.

The initial seed value for generating the formulas can be provided with `-s seed`. This option allows to run this script repeatedly with the exact same generated formulas. If this option is not provided, a random initial seed value will be used.

## Return values

- 0: all formulas completed without failure
- 1: stopped by SIGTERM
- 2: stopped by failed formula

## Output checks

The script generates a directory `outputDir` with subdirectories having the same name as the seed value. In each of this subdirectories, the generated formula is stored in `formula.cnf` and the outputs of the solvers in the other `*.cnf` files. Furthermore, a file `report.txt` is created that summarizes the results for the metrics: `success` if the solver yielded the expected result, `fail` if an unexpected result is observed, `timeout` if the solver did not complete within the provided timeout, `reftimeout` if the reference solver did not complete within the provided timeout.

A quick search for all failed formulas is simple with the `grep` command:
```
grep fail outputDir/*/report.txt
```
