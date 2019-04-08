// Origin author：https://blog.csdn.net/luzhanbo207/article/details/78819125 
// Halide tutorial lesson 7: Multi-stage pipelines
// Halide教程第七课： 多阶段流水线

// On linux, you can compile and run it like so:
// 在linux平台，按如下方式编译执行
// g++ lesson_07*.cpp -g -std=c++11 -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -o lesson_07
// LD_LIBRARY_PATH=../bin ./lesson_07

#include "Halide.h"
#include <stdio.h>

using namespace Halide;

// Support code for loading pngs.
#include "halide_image_io.h"
using namespace Halide::Tools;

int main(int argc, char **argv) {
    // First we'll declare some Vars to use below.
    Var x("x"), y("y"), c("c");

    // Now we'll express a multi-stage pipeline that blurs an image
    // first horizontally, and then vertically.
    // 我们表示一个图像模糊的多阶段流水线，首先沿水平方向模糊，接着沿垂直方向模糊
    {
        // Take a color 8-bit input
        Buffer<uint8_t> input = load_image("images/rgb.png");

        // Upgrade it to 16-bit, so we can do math without it overflowing.
        // 将输入数据升级到16比特，方式做数学计算时出现数据溢出
        Func input_16("input_16");
        input_16(x, y, c) = cast<uint16_t>(input(x, y, c));

        // Blur it horizontally:
        // 水平方向模糊
        Func blur_x("blur_x");
        blur_x(x, y, c) = (input_16(x-1, y, c) +
                           2 * input_16(x, y, c) +
                           input_16(x+1, y, c)) / 4;

        // Blur it vertically:
        // 垂直方向模糊
        Func blur_y("blur_y");
        blur_y(x, y, c) = (blur_x(x, y-1, c) +
                           2 * blur_x(x, y, c) +
                           blur_x(x, y+1, c)) / 4;

        // Convert back to 8-bit.
        // 图像数据转回8比特
        Func output("output");
        output(x, y, c) = cast<uint8_t>(blur_y(x, y, c));

        // Each Func in this pipeline calls a previous one using
        // familiar function call syntax (we've overloaded operator()
        // on Func objects). A Func may call any other Func that has
        // been given a definition. This restriction prevents
        // pipelines with loops in them. Halide pipelines are always
        // feed-forward graphs of Funcs.
        // 在这个流水线中的每个函数采用相似的语法调用前面的函数。一个函数可以调用其他已经被定义过的
        // 函数。这样的现实可以避免它们内部的循环。Halide的流水线是有函数构成的前向图结构。

        // Now let's realize it...

        // Buffer<uint8_t> result = output.realize(input.width(), input.height(), 3);

        // Except that the line above is not going to work. Uncomment
        // it to see what happens.

        // Realizing this pipeline over the same domain as the input
        // image requires reading pixels out of bounds in the input,
        // because the blur_x stage reaches outwards horizontally, and
        // the blur_y stage reaches outwards vertically. Halide
        // detects this by injecting a piece of code at the top of the
        // pipeline that computes the region over which the input will
        // be read. When it starts to run the pipeline it first runs
        // this code, determines that the input will be read out of
        // bounds, and refuses to continue. No actual bounds checks
        // occur in the inner loop; that would be slow.
        //  在同样的像素区域实现上述算法，需要input图像边界意外的像素。Halide会通过在最外层的pipeline注射
        // 一段代码来检查哪些input的像素数据将要被读取使用。当pipeline开始跑起来的时候，遇到输入数据超出
        // 边界，那么Halide会拒绝继续执行下去。内存循环没有边界检查，因此这样会导致程序很慢。
        // 
        // So what do we do? There are a few options. If we realize
        // over a domain shifted inwards by one pixel, we won't be
        // asking the Halide routine to read out of bounds. We saw how
        // to do this in the previous lesson:
        // 那么，如何解决这个问题呢？如果我们处理的区域所需要的input数据就没有超出input的边界，就没有这个
        // 问题了。一种可行的办法，是采用上节课所使用的移动执行区域的办法，输出图像数据行和列减少。
        Buffer<uint8_t> result(input.width()-2, input.height()-2, 3);
        result.set_min(1, 1);
        output.realize(result);

        // Save the result. It should look like a slightly blurry
        // parrot, and it should be two pixels narrower and two pixels
        // shorter than the input image.
        // 输出图像结果行和列均比input图像少2.
        save_image(result, "blurry_parrot_1.png");

        // This is usually the fastest way to deal with boundaries:
        // don't write code that reads out of bounds :) The more
        // general solution is our next example.
        // 这样的方法是处理是处理边界问题最快的方法。但是如果想保持输出图像和原始图像大小一致
        // 更常用的方法是下面的这个例子.
    }

    // The same pipeline, with a boundary condition on the input.
    {
        // Take a color 8-bit input
        Buffer<uint8_t> input = load_image("images/rgb.png");

        // This time, we'll wrap the input in a Func that prevents
        // reading out of bounds:
        // 将输入图像包裹在一个Func函数里面，防止图像访问像素点越界
        Func clamped("clamped");

        // Define an expression that clamps x to lie within the
        // range [0, input.width()-1].
        // 定义一个表达式，将x夹在[0, input.width()-1]闭区间里
        Expr clamped_x = clamp(x, 0, input.width()-1);
        // clamp(x, a, b) is equivalent to max(min(x, b), a).

        // Similarly clamp y.
        Expr clamped_y = clamp(y, 0, input.height()-1);
        // Load from input at the clamped coordinates. This means that
        // no matter how we evaluated the Func 'clamped', we'll never
        // read out of bounds on the input. This is a clamp-to-edge
        // style boundary condition, and is the simplest boundary
        // condition to express in Halide.
        // 从一个加紧限制的坐标中去读取input数据意味着无论如何去计算clamped函数，都不会越界读取input        

        clamped(x, y, c) = input(clamped_x, clamped_y, c);

        // Defining 'clamped' in that way can be done more concisely
        // using a helper function from the BoundaryConditions
        // namespace like so:
        // 上述定义clamped方法可以更简单地通过调用BoundaryConditions的成员函数达到目的
        // clamped = BoundaryConditions::repeat_edge(input);
        //
        // These are important to use for other boundary conditions,
        // because they are expressed in the way that Halide can best
        // understand and optimize. When used correctly they are as
        // cheap as having no boundary condition at all.
        // 通过BoundaryConditions类的方式调用边界条件很重要，因为halide可以很好的理解边界条件并优化它们
        // 正确使它们，可以向没有边界条件一样简单方便。

        // Upgrade it to 16-bit, so we can do math without it
        // overflowing. This time we'll refer to our new Func
        // 'clamped', instead of referring to the input image
        // directly.
        Func input_16("input_16");
        input_16(x, y, c) = cast<uint16_t>(clamped(x, y, c));

        // The rest of the pipeline will be the same...

        // Blur it horizontally:
        Func blur_x("blur_x");
        blur_x(x, y, c) = (input_16(x-1, y, c) +
                           2 * input_16(x, y, c) +
                           input_16(x+1, y, c)) / 4;

        // Blur it vertically:
        Func blur_y("blur_y");
        blur_y(x, y, c) = (blur_x(x, y-1, c) +
                           2 * blur_x(x, y, c) +
                           blur_x(x, y+1, c)) / 4;

        // Convert back to 8-bit.
        Func output("output");
        output(x, y, c) = cast<uint8_t>(blur_y(x, y, c));

        // This time it's safe to evaluate the output over the some
        // domain as the input, because we have a boundary condition.
        // 因为有边界条件的处理，可以安全的进行算法调用，输出结果的尺寸也和原始图像一致。
        Buffer<uint8_t> result = output.realize(input.width(), input.height(), 3);

        // Save the result. It should look like a slightly blurry
        // parrot, but this time it will be the same size as the
        // input.
        save_image(result, "blurry_parrot_2.png");
    }

    printf("Success!\n");
    return 0;
}

