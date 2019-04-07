# Halide_Turorial
This repository will introduce the basic of Halide -> (Algorithm + Scheduling)

## Basic Functions

Tile             |   Tile + Fuse + Parallel | Vectorize (SIMD) |  Unroll Loop
:-------------------------:|:-------------------------: |:-------------------------: |:-------------------------:
Divide whole image into tiles|  Taling parallel |   x86 SSE command |  Reduce repeat calculation
![](./figures/tile.gif?raw=true)  | ![](./figures/tile_parallel.gif?raw=true) | ![](./figures/vectorize.gif?raw=true) |  ![](./figures/unroll.gif?raw=true)


## Scheduling multi-stage pipelines

#### Stage 1: 
producer(x, y) = sin(x * y)

#### Stage 2: 
consumer(x, y) = (producer(x, y)   +
                  producer(x, y+1) +
                  producer(x+1, y) +
                  producer(x+1, y+1))/4









  
