# mccs OCaml library

mccs (which stands for Multi Criteria CUDF Solver) is a CUDF problem solver
developed at UNS during the European MANCOOSI project.

This repository contains a stripped-down version of the
[mccs solver](http://www.i3s.unice.fr/~cpjm/misc/mccs.html), taken from snapshot
1.1, with a binding as an OCaml library, and building with `jbuilder`.

This is a very early prototype, using a very dumb interface.

Build using `opam install .` (opam 2.0), or `jbuilder build`.

Note: this depends on flex, bison and a C++ compiler (only tested with g++).
