//vOrigin author：https://blog.csdn.net/luzhanbo207/article/details/78710831 \
// Halide教程第五课：向量化，并行化，平铺，数据分块
// 本课展示了如何才操作函数像素索引的计算顺序，包括向量化/并行化/平铺/分块等技术

// 在linux系统中，采用如下指令编译并执行
// g++ lesson_05*.cpp -g -I ../include -L ../bin -lHalide -lpthread -ldl -o lesson_05 -std=c++11
// LD_LIBRARY_PATH=../bin ./lesson_05

#include "Halide.h"
#include <stdio.h>
#include <algorithm>
using namespace Halide;

int main(int argc, char **argv) {

    Var x("x"), y("y");

    // First we observe the default ordering.
    {
        Func gradient("gradient");
        gradient(x, y) = x + y;
        gradient.trace_stores();

        //默认遍历像素的顺序是行优先，即内层循环沿着行方向，外层循环沿着列方向
        printf("Evaluating gradient row-major\n");
        Buffer<int> output = gradient.realize(4, 4);

        // The equivalent C is:
        printf("Equivalent C:\n");
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                printf("Evaluating at x = %d, y = %d: %d\n", x, y, x + y);
            }
        }
        printf("\n\n");

        // 跟踪系统调度可以很容易理解调度系统如何工作。可以通过Halide提供的函数来打印出实际工作
        // 是执行的哪种循环调度。
        printf("Pseudo-code for the schedule:\n");
        gradient.print_loop_nest();
        printf("\n");

        // Because we're using the default ordering, it should print:
        // compute gradient:
        //   for y:
        //     for x:
        //       gradient(...) = ...
    }

    // Reorder variables.
    {
        Func gradient("gradient_col_major");
        gradient(x, y) = x + y;
        gradient.trace_stores();

        // 可以通过reorder函数来改变函数遍历的顺序，下面的语句将行方向（y）置于内层循环，而将原本的内层
        // 循环调整到了外循环。也就是说y遍历比x遍历更快。是一种列优先的遍历方法
        gradient.reorder(y, x);

        printf("Evaluating gradient column-major\n");
        Buffer<int> output = gradient.realize(4, 4);

        printf("Equivalent C:\n");
        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                printf("Evaluating at x = %d, y = %d: %d\n", x, y, x + y);
            }
        }
        printf("\n");

        // 
        printf("Pseudo-code for the schedule:\n");
        gradient.print_loop_nest();
        printf("\n");
    }

    // Split a variable into two.
    {
        Func gradient("gradient_split");
        gradient(x, y) = x + y;
        gradient.trace_stores();

        // 原始调度中，最有效的就是split调度了，它将一个大循环，拆解成一个外部循环和一个内部循环；
        // 即，将x方向的循环，拆成一个外部循环x_outer和一个内部循环x_inner
        // 下面的split将x拆成x_outer,x_inner, 内循环的长度为2
        Var x_outer, x_inner;
        gradient.split(x, x_outer, x_inner, 2);

        printf("Evaluating gradient with x split into x_outer and x_inner \n");
        Buffer<int> output = gradient.realize(4, 4);

        printf("Equivalent C:\n");
        for (int y = 0; y < 4; y++) {
            for (int x_outer = 0; x_outer < 2; x_outer++) {
                for (int x_inner = 0; x_inner < 2; x_inner++) {
                    int x = x_outer * 2 + x_inner;
                    printf("Evaluating at x = %d, y = %d: %d\n", x, y, x + y);
                }
            }
        }
        printf("\n");

        printf("Pseudo-code for the schedule:\n");
        gradient.print_loop_nest();
        printf("\n");
    }

    // Fuse two variables into one.
    {
        Func gradient("gradient_fused");
        gradient(x, y) = x + y;

        // 和split相反的是fuse，它将两个变量融合成一个变量，fuse的重要性并没有split高。
        Var fused;
        gradient.fuse(x, y, fused);

        printf("Evaluating gradient with x and y fused\n");
        Buffer<int> output = gradient.realize(4, 4);

        printf("Equivalent C:\n");
        for (int fused = 0; fused < 4*4; fused++) {
            int y = fused / 4;
            int x = fused % 4;
            printf("Evaluating at x = %d, y = %d: %d\n", x, y, x + y);
        }
        printf("\n");

        printf("Pseudo-code for the schedule:\n");
        gradient.print_loop_nest();
        printf("\n");
    }

    // Evaluating in tiles.
    // tile的中文意思是瓦片，在这里是指将图像数据拆分成和瓦片一项的小图像块
    {
        Func gradient("gradient_tiled");
        gradient(x, y) = x + y;
        gradient.trace_stores();

        // 既然我们可以拆分和调整顺序，我们可以按照划分数据块的方式来进行计算。将x和y方向拆分，然后
        // 调制x和y的顺序，按照小的数据块的方式来进行遍历。
        // 一个小的数据块将整个图像划分成小的矩形，外层循环在tile击毙恩进行循环，遍历所有的tile。
        Var x_outer, x_inner, y_outer, y_inner;
        gradient.split(x, x_outer, x_inner, 4);
        gradient.split(y, y_outer, y_inner, 4);
        gradient.reorder(x_inner, y_inner, x_outer, y_outer);

        // This pattern is common enough that there's a shorthand for it:
        // gradient.tile(x, y, x_outer, y_outer, x_inner, y_inner, 4, 4);

        printf("Evaluating gradient in 4x4 tiles\n");
        Buffer<int> output = gradient.realize(8, 8);

        printf("Equivalent C:\n");
        for (int y_outer = 0; y_outer < 2; y_outer++) {
            for (int x_outer = 0; x_outer < 2; x_outer++) {
                for (int y_inner = 0; y_inner < 4; y_inner++) {
                    for (int x_inner = 0; x_inner < 4; x_inner++) {
                        int x = x_outer * 4 + x_inner;
                        int y = y_outer * 4 + y_inner;
                        printf("Evaluating at x = %d, y = %d: %d\n", x, y, x + y);
                    }
                }
            }
        }
        printf("\n");

        printf("Pseudo-code for the schedule:\n");
        gradient.print_loop_nest();
        printf("\n");
    }

    // Evaluating in vectors.
    {
        Func gradient("gradient_in_vectors");
        gradient(x, y) = x + y;
        gradient.trace_stores();

        // split能够让内层循环变量在一个划分银子内变化。这个划分因子通常是指定的，因此在编译时是一个常数
        // 因此我们可以调用向量化指令来执行内部循环。在这里我们指定这个因子为4，这样就可以调用x86机器上的SSE
        //指令来计算4倍宽的向量，这里充分利用了cpu的SIMD指令来加快计算
        Var x_outer, x_inner;
        gradient.split(x, x_outer, x_inner, 4);
        gradient.vectorize(x_inner);

        // 上述过程有更简单的形式
        // gradient.vectorize(x, 4);
        // 等价于
        // gradient.split(x, x, x_inner, 4);
        // gradient.vectorize(x_inner);
        // 这里我们重用了x，将它当作外循环变量，稍后的调度将x当作外循环(x_outer)来进行调度

        // 这次在一个8x4的矩形上执行gradient算法
        printf("Evaluating gradient with x_inner vectorized \n");
        Buffer<int> output = gradient.realize(8, 4);

        printf("Equivalent C:\n");
        for (int y = 0; y < 4; y++) {
            for (int x_outer = 0; x_outer < 2; x_outer++) {
                // The loop over x_inner has gone away, and has been
                // replaced by a vectorized version of the
                // expression. On x86 processors, Halide generates SSE
                // for all of this.
                int x_vec[] = {x_outer * 4 + 0,
                               x_outer * 4 + 1,
                               x_outer * 4 + 2,
                               x_outer * 4 + 3};
                int val[] = {x_vec[0] + y,
                             x_vec[1] + y,
                             x_vec[2] + y,
                             x_vec[3] + y};
                printf("Evaluating at <%d, %d, %d, %d>, <%d, %d, %d, %d>:"
                       " <%d, %d, %d, %d>\n",
                       x_vec[0], x_vec[1], x_vec[2], x_vec[3],
                       y, y, y, y,
                       val[0], val[1], val[2], val[3]);
            }
        }
        printf("\n");

        printf("Pseudo-code for the schedule:\n");
        gradient.print_loop_nest();
        printf("\n");
    }

    // Unrolling a loop.
    {
        Func gradient("gradient_unroll");
        gradient(x, y) = x + y;
        gradient.trace_stores();

        // 如果多个像素共享一些重复的（overlapping）数据，可以将循环铺平，从而共享的数据只需要载入或者
        // 计算一次。它和向量化的表达方式类似。先将数据进行划分，然后将内层循环铺平。
        Var x_outer, x_inner;
        gradient.split(x, x_outer, x_inner, 2);
        gradient.unroll(x_inner);

        // The shorthand for this is:
        // gradient.unroll(x, 2);

        printf("Evaluating gradient unrolled by a factor of two\n");
        Buffer<int> result = gradient.realize(4, 4);

        printf("Equivalent C:\n");
        for (int y = 0; y < 4; y++) {
            for (int x_outer = 0; x_outer < 2; x_outer++) {
                // Instead of a for loop over x_inner, we get two
                // copies of the innermost statement.
                {
                    int x_inner = 0;
                    int x = x_outer * 2 + x_inner;
                    printf("Evaluating at x = %d, y = %d: %d\n", x, y, x + y);
                }
                {
                    int x_inner = 1;
                    int x = x_outer * 2 + x_inner;
                    printf("Evaluating at x = %d, y = %d: %d\n", x, y, x + y);
                }
            }
        }
        printf("\n");

        printf("Pseudo-code for the schedule:\n");
        gradient.print_loop_nest();
        printf("\n");
    }

    // Splitting by factors that don't divide the extent.
    {
        Func gradient("gradient_split_7x2");
        gradient(x, y) = x + y;
        gradient.trace_stores();

        // 当原来图像尺寸不能整除划分的小矩形尺寸时，最后的一行或者一列的tile在边界处会出现重复计算的现象
        Var x_outer, x_inner;
        gradient.split(x, x_outer, x_inner, 3);

        printf("Evaluating gradient over a 7x2 box with x split by three \n");
        Buffer<int> output = gradient.realize(7, 2);

        printf("Equivalent C:\n");
        for (int y = 0; y < 2; y++) {
            for (int x_outer = 0; x_outer < 3; x_outer++) { // Now runs from 0 to 2
                for (int x_inner = 0; x_inner < 3; x_inner++) {
                    int x = x_outer * 3;
                    // Before we add x_inner, make sure we don't
                    // evaluate points outside of the 7x2 box. We'll
                    // clamp x to be at most 4 (7 minus the split
                    // factor).
                    if (x > 4) x = 4;
                    x += x_inner;
                    printf("Evaluating at x = %d, y = %d: %d\n", x, y, x + y);
                }
            }
        }
        printf("\n");

        printf("Pseudo-code for the schedule:\n");
        gradient.print_loop_nest();
        printf("\n");

        // 如果仔细查看程序的输出，你会发现有些像素点进行了不止一次计算。这是因为尺寸不能整除所致
        // 由于Halide函数没有边缘效应，因此计算多次并不会产生副作用。
    }

    // Fusing, tiling, and parallelizing.
    {
        // 这里才是fuse真正发挥威力的地方。如果想要在多个维度进行并行计算，
        // 可以将多个循环网fuse起来，然后在fuse后的维度下进行并行计算

        Func gradient("gradient_fused_tiles");
        gradient(x, y) = x + y;
        gradient.trace_stores();

        Var x_outer, y_outer, x_inner, y_inner, tile_index;
        gradient.tile(x, y, x_outer, y_outer, x_inner, y_inner, 4, 4);
        gradient.fuse(x_outer, y_outer, tile_index);
        gradient.parallel(tile_index);

        // 每个调度函数返回的是引用类型，因此可以按如下方式用点号连接起多次调用
        // gradient
        //     .tile(x, y, x_outer, y_outer, x_inner, y_inner, 2, 2)
        //     .fuse(x_outer, y_outer, tile_index)
        //     .parallel(tile_index);


        printf("Evaluating gradient tiles in parallel\n");
        Buffer<int> output = gradient.realize(8, 8);

        // tile层面的调度是乱序的，但是在每一个tile内部，是行优先的计算顺序

        printf("Equivalent (serial) C:\n");
        // This outermost loop should be a parallel for loop, but that's hard in C.
        for (int tile_index = 0; tile_index < 4; tile_index++) {
            int y_outer = tile_index / 2;
            int x_outer = tile_index % 2;
            for (int y_inner = 0; y_inner < 4; y_inner++) {
                for (int x_inner = 0; x_inner < 4; x_inner++) {
                    int y = y_outer * 4 + y_inner;
                    int x = x_outer * 4 + x_inner;
                    printf("Evaluating at x = %d, y = %d: %d\n", x, y, x + y);
                }
            }
        }
        printf("\n");

        printf("Pseudo-code for the schedule:\n");
        gradient.print_loop_nest();
        printf("\n");
    }

    // 将前面演示的调度综合在一起
    {
        Func gradient_fast("gradient_fast");
        gradient_fast(x, y) = x + y;

        // tile尺寸为64x64，采用并行计算
        Var x_outer, y_outer, x_inner, y_inner, tile_index;
        gradient_fast
            .tile(x, y, x_outer, y_outer, x_inner, y_inner, 64, 64)
            .fuse(x_outer, y_outer, tile_index)
            .parallel(tile_index);

        // 将内部的64x64tile继续拆分成更小的tile。在x方向上采用向量化的计算（调用SIMD指令），
        // 在y方向进行平铺
        Var x_inner_outer, y_inner_outer, x_vectors, y_pairs;
        gradient_fast
            .tile(x_inner, y_inner, x_inner_outer, y_inner_outer, x_vectors, y_pairs, 4, 2)
            .vectorize(x_vectors)
            .unroll(y_pairs);

        Buffer<int> result = gradient_fast.realize(350, 250);

        printf("Checking Halide result against equivalent C...\n");
        for (int tile_index = 0; tile_index < 6 * 4; tile_index++) {
            int y_outer = tile_index / 4;
            int x_outer = tile_index % 4;
            for (int y_inner_outer = 0; y_inner_outer < 64/2; y_inner_outer++) {
                for (int x_inner_outer = 0; x_inner_outer < 64/4; x_inner_outer++) {
                    // We're vectorized across x
                    int x = std::min(x_outer * 64, 350-64) + x_inner_outer*4;
                    int x_vec[4] = {x + 0,
                                    x + 1,
                                    x + 2,
                                    x + 3};

                    // And we unrolled across y
                    int y_base = std::min(y_outer * 64, 250-64) + y_inner_outer*2;
                    {
                        // y_pairs = 0
                        int y = y_base + 0;
                        int y_vec[4] = {y, y, y, y};
                        int val[4] = {x_vec[0] + y_vec[0],
                                      x_vec[1] + y_vec[1],
                                      x_vec[2] + y_vec[2],
                                      x_vec[3] + y_vec[3]};

                        // Check the result.
                        for (int i = 0; i < 4; i++) {
                            if (result(x_vec[i], y_vec[i]) != val[i]) {
                                printf("There was an error at %d %d!\n",
                                       x_vec[i], y_vec[i]);
                                return -1;
                            }
                        }
                    }
                    {
                        // y_pairs = 1
                        int y = y_base + 1;
                        int y_vec[4] = {y, y, y, y};
                        int val[4] = {x_vec[0] + y_vec[0],
                                      x_vec[1] + y_vec[1],
                                      x_vec[2] + y_vec[2],
                                      x_vec[3] + y_vec[3]};

                        // Check the result.
                        for (int i = 0; i < 4; i++) {
                            if (result(x_vec[i], y_vec[i]) != val[i]) {
                                printf("There was an error at %d %d!\n",
                                       x_vec[i], y_vec[i]);
                                return -1;
                            }
                        }
                    }
                }
            }
        }
        printf("\n");

        printf("Pseudo-code for the schedule:\n");
        gradient_fast.print_loop_nest();
        printf("\n");

        // Note that in the Halide version, the algorithm is specified
        // once at the top, separately from the optimizations, and there
        // aren't that many lines of code total. Compare this to the C
        // version. There's more code (and it isn't even parallelized or
        // vectorized properly). More annoyingly, the statement of the
        // algorithm (the result is x plus y) is buried in multiple places
        // within the mess. This C code is hard to write, hard to read,
        // hard to debug, and hard to optimize further. This is why Halide
        // exists.
        // 从前面给出的几个调度的例子可以看出，Halide对应版本的代码相对于C的代码，算法和调度分别进行分离
        // 调度方便，发ingbianjinxing游动优化，而对应的C语言代码，冗长杂乱，很难写/难读/调试困难，
        // 不方便进一步优化
    }


    printf("Success!\n");
    return 0;
}

