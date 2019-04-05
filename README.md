# Halide_Turorial
This repository will introduce the basic of Halide -> (Algorithm + Scheduling)

## Basic Functions

Tile             |   Tile + Fuse + Parallel | Vectorize (SIMD) |  Unroll Loop
:-------------------------:|:-------------------------: |:-------------------------: |:-------------------------:
Divide whole image into tiles|  parallel taling |   x86 SSE command |  Reduce repeat calculation
![](./figures/tile.gif?raw=true)  | ![](./figures/tile_parallel.gif?raw=true) | ![](./figures/vectorize.gif?raw=true) |  ![](./figures/unroll.gif?raw=true)


## Scheduling multi-pipelines



compute_root             |   compute_at | store_root.compute_at |  Tiling + compute_at
:-------------------------:|:-------------------------: |:-------------------------: |:-------------------------:
compute all producer first|  compute producer inside y loop |   use scanline to store intermediate data |  divide compute_at into tiles
![](./figures/compute_root.gif?raw=true)  | ![](./figures/computer_at.gif?raw=true) | ![](./figures/root_at.gif?raw=true) |  ![](./figures/tile_at.gif?raw=true)








  
