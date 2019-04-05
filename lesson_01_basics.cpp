// Origin author https://blog.csdn.net/luzhanbo207/article/details/78655484 
// Halide tutorial lesson 1: Getting started with Funcs, Vars, and Exprs
// Halide入门教程第一课：了解Funcs（函数），Vars（变量）和Exprs（表达式）

// This lesson demonstrates basic usage of Halide as a JIT compiler for imaging.
// 本课演示了Halide作为图像处理JIT compiler（即时编译器）的一些基本用法

// On linux, you can compile and run it like so:
// 在linux操作系统上，你可以按照如下方式进行编译和运行
// g++ lesson_01*.cpp -g -I ../include -L ../bin -lHalide -lpthread -ldl -o lesson_01 -std=c++11
// LD_LIBRARY_PATH=../bin ./lesson_01

// On os x:
// g++ lesson_01*.cpp -g -I ../include -L ../bin -lHalide -o lesson_01 -std=c++11
// DYLD_LIBRARY_PATH=../bin ./lesson_01

// If you have the entire Halide source tree, you can also build it by
// 如果你有整个Halide代码树，你可以按照如下方式进行编译
// running:
//    make tutorial_lesson_01_basics
// in a shell with the current directory at the top of the halide
// source tree.

// The only Halide header file you need is Halide.h. It includes all of Halide.
// Halide.h包含了整个Halide, 只需要include这个头文件即可
#include "Halide.h"

// We'll also include stdio for printf.
#include <stdio.h>

int main(int argc, char **argv) {

    // This program defines a single-stage imaging pipeline that
    // outputs a grayscale diagonal gradient.

    // A 'Func' object represents a pipeline stage. It's a pure
    // function that defines what value each pixel should have. You
    // can think of it as a computed image.
    // Func对象表示了一个pipeline阶段。它是一个纯函数，定义了每个像素点对应的值。
    // 可以理解为要进行计算的图像。
    Halide::Func gradient;

    // Var objects are names to use as variables in the definition of
    // a Func. They have no meaning by themselves.
    // Var对象是Func的定义域，或者说是Func的参数。它们本身没有任何意义。Var用来索引
    // 函数（图像）对应的像素点
    Halide::Var x, y;

    // We typically use Vars named 'x' and 'y' to correspond to the x
    // and y axes of an image, and we write them in that order. If
    // you're used to thinking of images as having rows and columns,
    // then x is the column index, and y is the row index.
    // x和y分别对应着图像的x轴和y轴，x对应的是列索引，y对应着行索引
    // -------------> x axes
    // |
    // |
    // |
    // \/ y axes

    // Funcs are defined at any integer coordinate of its variables as
    // an Expr in terms of those variables and other functions.
    // Here, we'll define an Expr which has the value x + y. Vars have
    // appropriate operator overloading so that expressions like
    // 'x + y' become 'Expr' objects.
    // 函数定义在整数坐标处，函数值是变量和其他函数的表达式的结果。
    // 在此我们定义了一个 x + y的表达式。变量重载了算数运算符，因此变量进行运算的结果是一个Expr对象
    Halide::Expr e = x + y;

    // Now we'll add a definition for the Func object. At pixel x, y,
    // the image will have the value of the Expr e. On the left hand
    // side we have the Func we're defining and some Vars. On the right
    // hand side we have some Expr object that uses those same Vars.
    // 现在我们将给函数对象一个定义的实现。在像素点x，y处，图像的像素值为表达式e的结果。
    // 函数对象和一些变量位于表达式的左边，表达式的右边是一些Expr对象。
    // gradient(x, y) = e 相当于在x，y处的像素值是表达式x+y的运算结果。
    gradient(x, y) = e;

    // This is the same as writing:
    //
    //   gradient(x, y) = x + y;
    //
    // which is the more common form, but we are showing the
    // intermediate Expr here for completeness.

    // That line of code defined the Func, but it didn't actually
    // compute the output image yet. At this stage it's just Funcs,
    // Exprs, and Vars in memory, representing the structure of our
    // imaging pipeline. We're meta-programming. This C++ program is
    // constructing a Halide program in memory. Actually computing
    // pixel data comes next.
    // 上述几行代码定义了Func，但实际上并没有计算输出图像。在这个阶段，它仅仅是内存函数，表达式和变量，
    // 我们在进行元编程。C++程序正在内存中构造Halide程序。实际上进行像素数据计算的在下一阶段进行。

    // Now we 'realize' the Func, which JIT compiles some code that
    // implements the pipeline we've defined, and then runs it.  We
    // also need to tell Halide the domain over which to evaluate the
    // Func, which determines the range of x and y above, and the
    // resolution of the output image. Halide.h also provides a basic
    // templatized image type we can use. We'll make an 800 x 600
    // image.
    // 在此，我们realize前一个阶段定义的Func，即时编译器编译我们定义的算法流程代码，然后运行代码。
    // 我们需要告诉Halide在在指定的区域进行计算。这里的domain可以理解为像素点的范围。domain决定了
    // 前面的x，y变量的范围，输出图像的分辨率等。Halide.h提供了供使用的一些基本的图像类型模板。
    // 在此，我们是哦用一个800x600的图像。
    Halide::Buffer<int32_t> output = gradient.realize(800, 600);

    // Halide does type inference for you. Var objects represent
    // 32-bit integers, so the Expr object 'x + y' also represents a
    // 32-bit integer, and so 'gradient' defines a 32-bit image, and
    // so we got a 32-bit signed integer image out when we call
    // 'realize'. Halide types and type-casting rules are equivalent
    // to C.
    // Halide能够进行数据类型推断。Var对象由32位（有符号）整数表示，Expr对象x+y也是32位有符号整数
    // 因此，gradient定义了一幅32位的图像。在我们调用了realize之后，得到一幅32为有符号整数图像输出。
    // Halide的类型转换规则和C语言的类型转换一致。

    // Let's check everything worked, and we got the output we were
    // expecting:
    for (int j = 0; j < output.height(); j++) {
        for (int i = 0; i < output.width(); i++) {
            // We can access a pixel of an Buffer object using similar
            // syntax to defining and using functions.
            if (output(i, j) != i + j) {
                printf("Something went wrong!\n"
                       "Pixel %d, %d was supposed to be %d, but instead it's %d\n",
                       i, j, i+j, output(i, j));
                return -1;
            }
        }
    }

    // Everything worked! We defined a Func, then called 'realize' on
    // it to generate and run machine code that produced an Buffer.
    printf("Success!\n");

    return 0;
}

