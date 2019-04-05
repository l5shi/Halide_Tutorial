// Origin author：https://blog.csdn.net/luzhanbo207/article/details/78689444 

// Halide tutorial lesson 4: Debugging with tracing, print, and print_when
// Halide入门第四课：用tracing，print，print_when调试

// This lesson demonstrates how to follow what Halide is doing at runtime.
// 本课展示了如何跟踪Halide在运行时的行为

// On linux, you can compile and run it like so:
// g++ lesson_04*.cpp -g -I ../include -L ../bin -lHalide -lpthread -ldl -o lesson_04 -std=c++11
// LD_LIBRARY_PATH=../bin ./lesson_04

// On os x:
// g++ lesson_04*.cpp -g -I ../include -L ../bin -lHalide -o lesson_04 -std=c++11
// DYLD_LIBRARY_PATH=../bin ./lesson_04

// If you have the entire Halide source tree, you can also build it by
// running:
//    make tutorial_lesson_04_debugging_2
// in a shell with the current directory at the top of the halide
// source tree.

#include "Halide.h"
#include <stdio.h>
using namespace Halide;

int main(int argc, char **argv) {

    Var x("x"), y("y");

    // Printing out the value of Funcs as they are computed.
    // 打印函数（Func）在计算时刻的值
    {
        // We'll define our gradient function as before.
        Func gradient("gradient");
        gradient(x, y) = x + y;

        // And tell Halide that we'd like to be notified of all
        // evaluations.
        // 告诉Halide我们想要跟踪所有的函数计算值
        gradient.trace_stores();

        // Realize the function over an 8x8 region.
        printf("Evaluating gradient\n");
        Buffer<int> output = gradient.realize(8, 8);

        // This will print out all the times gradient(x, y) gets
        // evaluated.

        // Now that we can snoop on what Halide is doing, let's try our
        // first scheduling primitive. We'll make a new version of
        // gradient that processes each scanline in parallel.
        Func parallel_gradient("parallel_gradient");
        parallel_gradient(x, y) = x + y;

        // We'll also trace this function.
        parallel_gradient.trace_stores();

        // Things are the same so far. We've defined the algorithm, but
        // haven't said anything about how to schedule it. In general,
        // exploring different scheduling decisions doesn't change the code
        // that describes the algorithm.
        // 在Halide中，由于算法和调度解耦合，算法的调度并不影响算法的描述

        // Now we tell Halide to use a parallel for loop over the y
        // coordinate. On Linux we run this using a thread pool and a task
        // queue. On OS X we call into grand central dispatch, which does
        // the same thing for us.
        // 在y方形并行执行for循环
        parallel_gradient.parallel(y);

        // This time the printfs should come out of order, because each
        // scanline is potentially being processed in a different
        // thread. The number of threads should adapt to your system, but
        // on linux you can control it manually using the environment
        // variable HL_NUM_THREADS.
        // 由于采用了并行计算饭，每行的计算可能位于不同的线程，因此输出结果可能会是乱序的。
        // 可以通过环境变量HL_NUM_THREADS来指定parallel的线程数
        printf("\nEvaluating parallel_gradient\n");
        parallel_gradient.realize(8, 8);
    }

    // Printing individual Exprs.
    {
        // trace_stores() can only print the value of a
        // Func. Sometimes you want to inspect the value of
        // sub-expressions rather than the entire Func. The built-in
        // function 'print' can be wrapped around any Expr to print
        // the value of that Expr every time it is evaluated.
        // trace_stores()函数打印函数值，内置的print函数可以答应表达式（Expr）对象的值

        // For example, say we have some Func that is the sum of two terms:
        Func f;
        f(x, y) = sin(x) + cos(y);

        // If we want to inspect just one of the terms, we can wrap
        // 'print' around it like so:
        // 如果我们仅仅需要关注表达式中的一个条目，我们可以在这个条目上加上print函数
        Func g;
        g(x, y) = sin(x) + print(cos(y));

        printf("\nEvaluating sin(x) + cos(y), and just printing cos(y)\n");
        g.realize(4, 4);
    }

    // Printing additional context.
    {
        // print can take multiple arguments. It prints all of them
        // and evaluates to the first one. The arguments can be Exprs
        // or constant strings. This can be used to print additional
        // context alongside the value:
        // 如果需要，可以在打印单个条目上加上额外的文本
        Func f;
        f(x, y) = sin(x) + print(cos(y), "<- this is cos(", y, ") when x =", x);

        printf("\nEvaluating sin(x) + cos(y), and printing cos(y) with more context\n");
        f.realize(4, 4);

        // It can be useful to split expressions like the one above
        // across multiple lines to make it easier to turn on and off
        // printing certain values while debugging.
        Expr e = cos(y);
        // Uncomment the following line to print the value of cos(y)
        // e = print(e, "<- this is cos(", y, ") when x =", x);
        Func g;
        g(x, y) = sin(x) + e;
        g.realize(4, 4);
    }

    // Conditional printing
    {
        // Both print and trace_stores can produce a lot of output. If
        // you're looking for a rare event, or just want to see what
        // happens at a single pixel, this amount of output can be
        // difficult to dig through. Instead, the function print_when
        // can be used to conditionally print an Expr. The first
        // argument to print_when is a boolean Expr. If the Expr
        // evaluates to true, it returns the second argument and
        // prints all of the arguments. If the Expr evaluates to false
        // it just returns the second argument and does not print.
        // 如果需要查看中间某个特定的结果，可以调用条件打印函数，打印出在特定条件下，表达式的结果。
        // print_when(bool_expr, expr, context)
        // 如果 bool_expr == ture: 返回expr，打印context内容
        // 否则只返回expr

        Func f;
        Expr e = cos(y);
        e = print_when(x == 37 && y == 42, e, "<- this is cos(y) at x, y == (37, 42)");
        f(x, y) = sin(x) + e;
        printf("\nEvaluating sin(x) + cos(y), and printing cos(y) at a single pixel\n");
        f.realize(640, 480);

        // print_when can also be used to check for values you're not expecting:
        Func g;
        e = cos(y);
        e = print_when(e < 0, e, "cos(y) < 0 at y ==", y);
        g(x, y) = sin(x) + e;
        printf("\nEvaluating sin(x) + cos(y), and printing whenever cos(y) < 0\n");
        g.realize(4, 4);
    }

    // Printing expressions at compile-time.
    {
        // The code above builds up a Halide Expr across several lines
        // of code. If you're programmatically constructing a complex
        // expression, and you want to check the Expr you've created
        // is what you think it is, you can also print out the
        // expression itself using C++ streams:
        // 在编写一些复杂的表达式时，如果你想要查看表达式是否和你想象中一样，可以用c++
        // 的输出流将表达式结果打印到标准输出上，检查是否如预期一致。
        Var fizz("fizz"), buzz("buzz");
        Expr e = 1;
        for (int i = 2; i < 100; i++) {
            if (i % 3 == 0 && i % 5 == 0) e += fizz*buzz;
            else if (i % 3 == 0) e += fizz;
            else if (i % 5 == 0) e += buzz;
            else e += i;
        }
        std::cout << "Printing a complex Expr: " << e << "\n";
    }

    printf("Success!\n");
    return 0;
}