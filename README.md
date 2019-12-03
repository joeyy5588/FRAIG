# FRAIG

## Introduction
FRAIG (Functionally Reduced And-Inverter Graph), is a tool that can reduce redundant logic gates in a circuit.

## Usage
```
$ make clean
$ make
$ ./fraig
```

## Command 
- Sweep: Delete unsed gates
- Optimize: Perform simple optimization
- Strash: Structural hash
- Simulate: Use binary pattern to perform the simulation
- Fraig: Use SAT solver to solve potential equivalent gates
