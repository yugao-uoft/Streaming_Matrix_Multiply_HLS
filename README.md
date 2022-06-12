# Streaming Matrix-Multiplication
streaming matrix multiplication kernel written in HLS

## Build Process
- Build a Vivado hls project using source files in weights_gen/src
- Run csim to generate weights.hpp
- Build a Vivado hls project using source files in matrixmultiply/src
