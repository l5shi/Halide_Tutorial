![](https://img.shields.io/badge/language-C++-orange.svg)
[![](https://img.shields.io/badge/常联系-click_for_contact-green.svg)](https://github.com/l5shi/__Overview__/blob/master/thanks/README.md)
[![](https://img.shields.io/badge/Donate-支付宝|微信|Venmo-blue.svg)](https://github.com/l5shi/__Overview__/blob/master/thanks/README.md)
[![](https://img.shields.io/badge/reference-padge-yellow.svg)](https://blog.csdn.net/luzhanbo207/article/details/78655484 )


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








 store_root.compute_at |  Tiling + compute_at
:-------------------------: |:-------------------------:
 Store intermediate data in several scanlines |  Divide compute_at into tiles
![](./figures/root_at.gif?raw=true) |  ![](./figures/tile_at.gif?raw=true)


compute_root             |   compute_at 
:-------------------------:|:-------------------------: 
Compute all producer before use|  Compute producer inside y loop 
![](./figures/compute_root.gif?raw=true)  | ![](./figures/compute_at.gif?raw=true) 



 
