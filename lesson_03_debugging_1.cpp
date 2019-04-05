// Origin author: https://blog.csdn.net/luzhanbo207/article/details/78673885 
// Halide tutorial lesson 3: Inspecting the generated code
// Halide入门第三课：检测生成代码

// This lesson demonstrates how to inspect what the Halide compiler is producing.
// 本课揭示了怎样查看Halide编译器做了些什么

// On linux, you can compile and run it like so:
// linux操作系统，按如下操作编译和运行
// g++ lesson_03*.cpp -g -I ../include -L ../bin -lHalide -lpthread -ldl -o lesson_03 -std=c++11
// LD_LIBRARY_PATH=../bin ./lesson_03

// On os x:
// g++ lesson_03*.cpp -g -I ../include -L ../bin -lHalide -o lesson_03 -std=c++11
// DYLD_LIBRARY_PATH=../bin ./lesson_03

// If you have the entire Halide source tree, you can also build it by
// running:
//    make tutorial_lesson_03_debugging_1
// in a shell with the current directory at the top of the halide
// source tree.

#include "Halide.h"
#include <stdio.h>

// This time we'll just import the entire Halide namespace
using namespace Halide;

int main(int argc, char **argv) {

    // We'll start by defining the simple single-stage imaging
    // pipeline from lesson 1.

    // This lesson will be about debugging, but unfortunately in C++,
    // objects don't know their own names, which makes it hard for us
    // to understand the generated code. To get around this, you can
    // pass a string to the Func and Var constructors to give them a
    // name for debugging purposes.
    // 本课主要陈述调试（debugging），但c++中的对象并没有自己的名字标签，这就给理解生成代码增加了困难
    // 为了克服这个问题，你可以给Func和Var的构造函数传入一个string类型的名字，从而达到方便调试的目的
    Func gradient("gradient");
    Var x("x"), y("y");
    gradient(x, y) = x + y;

    // Realize the function to produce an output image. We'll keep it
    // very small for this lesson.
    Buffer<int> output = gradient.realize(8, 8);

    // That line compiled and ran the pipeline. Try running this
    // lesson with the environment variable HL_DEBUG_CODEGEN set to
    // 1. It will print out the various stages of compilation, and a
    // pseudocode representation of the final pipeline.
    // 设置环境变量HL_DEBUG_CODEGEN=1，此时运行程序会打印出编译的不同阶段和最终pipeline的伪代码
    // export HL_DEBUG_CODEGEN=1

    // If you set HL_DEBUG_CODEGEN to a higher number, you can see
    // more and more details of how Halide compiles your pipeline.
    // Setting HL_DEBUG_CODEGEN=2 shows the Halide code at each stage
    // of compilation, and also the llvm bitcode we generate at the
    // end.
    // 设置HL_DEBUG_CODEGEN=2，此时会输出Halide编译的各个不同阶段，而且会输出llvm最终生成的字节码
    // export HL_DEBUG_CODEGEN=2

    // Halide will also output an HTML version of this output, which
    // supports syntax highlighting and code-folding, so it can be
    // nicer to read for large pipelines. Open gradient.html with your
    // browser after running this tutorial.
    // Halide也提供HTML形式的伪代码输出，支持语法高亮，代码折叠，翻遍大规模复杂pipeline的阅读
    gradient.compile_to_lowered_stmt("gradient.html", {}, HTML);

    // You can usually figure out what code Halide is generating using
    // this pseudocode. In the next lesson we'll see how to snoop on
    // Halide at runtime.

    printf("Success!\n");
    return 0;
}
