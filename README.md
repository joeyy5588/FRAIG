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
- Simulate: use binary pattern to simulate the whole circuit, do not reduce gates
- Fraig: use SAT solver to solve potential equivalent gates
