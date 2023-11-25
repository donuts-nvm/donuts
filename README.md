# dOnuts
DONUTS Implementation for Sniper Multi-Core Simulator

This is the source code for DONUTS (DON’t paUse The perSistence),
a checkpointing mechanism that provides crash consistency for
computing systems based on Non-Volatile Memories (NVM).

DONUTS is being developed by Kleber Kruger at State University
of Campinas and Federal University of Mato Grosso do Sul, Brazil.
This project was implemented in the Sniper multicore simulator
(described in the other README file).

- Some new features of this project:

* Support for write-through caches at all memory hierarchy levels
  (fixed lock bug in L2 caches onwards)
* Support for write-buffer in cache controller
* Non-Volatile Memory (NVM)
* Epoch system
* New cache replacement policy LRU-R (Least Recently Used Read)
* Python scripts for performance analysis

Please refer to the NOTICE file in the top-level directory for
licensing and copyright information.

- Reference Article:
  Kruger, K, Pannain, R, Azevedo, R. DONUTS: An efficient method for
  checkpointing in non-volatile memories. Concurrency Computat Pract Exper.
  2023; 35(18):e7574.
  https://doi.org/10.1002/cpe.7574

- Results of the published articles are stored in the results folder
  in the top-level directory:
  https://github.com/donuts-nvm/donuts/tree/main/results
