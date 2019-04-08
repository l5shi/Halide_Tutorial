// Origin author：https://blog.csdn.net/luzhanbo207/article/details/78711218 
// Halide tutorial lesson 6: Realizing Funcs over arbitrary domains
// Halide入门教程第六课：在指定区域上执行函数

// This lesson demonstrates how to evaluate a Func over a domain that
// does not start at (0, 0).
// 本课演示了如何在指定区域上执行函数，而不是默认的以（0，0）起点在整个图像上执行操作

// On linux, you can compile and run it like so:
// g++ lesson_06*.cpp -g -I ../include -L ../bin -lHalide -lpthread -ldl -o lesson_06 -std=c++11
// LD_LIBRARY_PATH=../bin ./lesson_06

#include "Halide.h"
#include <stdio.h>

using namespace Halide;

int main(int argc, char **argv) {

    // We define our familiar gradient function.
    Func gradient("gradient");
    Var x("x"), y("y");
    gradient(x, y) = x + y;

    // And turn on tracing so we can see how it is being evaluated.
    gradient.trace_stores();

    // 之前，按照如下方式进行梯度计算
    // gradient.realize(8, 8);
    // 它隐含地做了如下三件事
    // 1) 生成可以在任何矩形上进行计算的代码
    // 2) 分配一个新的8x8的图像存储空间
    // 3) 遍历x, y从(0, 0) 到 (7, 7) 把生成的结果保存到图像中
    // 4) 返回结果图像

    // 如果不想Halide分配新的图像空间，可以采用其他的方式调用realize成员函数。将图像以参数的形式传给realize
    // 将结果填充到对应的参数图像当中。下面的例子就是将计算结果填充到一幅已经存在的图像当中。
    printf("Evaluating gradient from (0, 0) to (7, 7)\n");
    Buffer<int> result(8, 8);
    gradient.realize(result);

    // Let's check it did what we expect:
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (result(x, y) != x + y) {
                printf("Something went wrong!\n");
                return -1;
            }
        }
    }

    // 现在，想要在5x7的矩形区域上计算梯度，而且起始坐标为（100， 50）
    // We start by creating an image that represents that rectangle:
    // 创建一个表示5x7图像的矩形区域
    Buffer<int> shifted(5, 7); // 在构造函数中指定尺寸
    shifted.set_min(100, 50); // 指定，计算区域的其实坐标（左上角坐标）

    printf("Evaluating gradient from (100, 50) to (104, 56)\n");

    // 这里不需要重新编译代码，原因是halide生成的是可以在任何矩形上操作的代码
    gradient.realize(shifted);

    // From C++, we also access the image object using coordinates
    // that start at (100, 50).
    for (int y = 50; y < 57; y++) {
        for (int x = 100; x < 105; x++) {
            if (shifted(x, y) != x + y) {
                printf("Something went wrong!\n");
                return -1;
            }
        }
    }

    // 如果想要在非矩形的区域上操作呢？非常遗憾，Halide值提供了矩形区域的操作。

    printf("Success!\n");
    return 0;
}

