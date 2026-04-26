# DAA Practice - MS-CFLP-CI Metaheuristics

# Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities

Implementation of several metaheuristics to solve the MS-CFLP-CI problem.

## Purpose

The main objective of this project is to study and implement different approaches for solving a NP-HARD problem, focusing on:

* Greedy algorithms as baseline solutions
* GRASP metaheuristics
* Local Search techniques (VND and RVND)
* General Variable Neighborhood Search (GVNS)
* Reinforcement Learning applied to neighborhood selection
* Design and implementation of neighborhood structures
* Efficient memory management using references and pointers in C++
* Good software design using design patterns and SOLID principles, allowing easy extension and maintenance of the system
* Performance analysis (solution quality vs execution time)

## Project Directory Structure

The project is fully implemented in C++ and organized in a modular way:

```text
.
├── Makefile
├── README.md
├── inputs
│   └── wlpXX.dzn               # Problem instances
├── outputs
│   ├── *.txt                   # Execution results and experiments
└── src
    ├── algorythms              # Solvers (Greedy, GRASP, VND, RVND, GVNS-RL)
    ├── explorers               # Neighborhood structures
    ├── instances               # Instance parsing and representation
    ├── perturbators            # GVNS perturbation strategies
    ├── solutions               # Solution representation
    └── main.cc                 # Entry point
```

## Implemented Algorithms

The following approaches have been implemented:

* Greedy Solver
* Fast baseline solution without improvement phase.
* GRASP + VND
* Constructive phase + deterministic local search.
* GRASP + RVND
* Same as VND but with randomized neighborhood selection.
* GRASP + GVNS-RL
* Advanced approach combining:
  - Perturbation (shift-based)
  - VND with Reinforcement Learning (ε-greedy policy)

## Building and Running the code

To compile the project, from the root directory run ```make```

To run an instance:

```bash
./practice5_DAA inputs/wlp01.dzn greedy
./practice5_DAA inputs/wlp01.dzn grasp-vnd
./practice5_DAA inputs/wlp01.dzn grasp-rvnd
./practice5_DAA inputs/wlp01.dzn grasp-gvns-rl
```

## Notes

The project has been heavily optimized to reduce execution time.

Initially, performance was much worse due to excessive memory copies, which was improved by:
* Using references and pointers
* Avoiding unnecessary solution duplication
* Using deltas instead of calculating the objective value from scratch every time.

## Author

Javier Acosta Portocarrero
alu0101660769@ull.edu.es / javier.acosta

## License

This project is intended for academic purposes.

You are free to read and learn from the code, but copying, modifying or redistributing it without permission is not allowed.

Contact the author for any other use.