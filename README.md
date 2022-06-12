# Streaming Matrix-Multiplication
Streaming matrix multiplication kernel written in HLS. 

One matrix is kept in the FPGA on-chip memory, the other one is streamed into the kernel through AXI-Stream.

## Build Process
- Build a Vivado hls project using source files in weights_gen/src
- Run csim to generate weights.hpp
- Build a Vivado hls project using source files in matrixmultiply/src
