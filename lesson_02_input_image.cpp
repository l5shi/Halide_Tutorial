// Origin author：https://blog.csdn.net/luzhanbo207/article/details/78664847 
// Halide tutorial lesson 2: Processing images
// Halide入门第二课： 处理图像

// This lesson demonstrates how to pass in input images and manipulate
// them.
// 本课展示了如何读入图像数据，并操作像素

// On linux, you can compile and run it like so:
// 在linux操作系统，你可以按照如下方式编译和运行该代码
// 运行之前确保操作系统安装了libpng和libjpeg库，可以到对应的sourceforge上找到对应的代码编译安装,
// 或者采用linux系统的包管理系统进行安装，debian系操作系统
// sudo apt-get install libpng
// sudo apt-get install libjpeg
// g++ lesson_02*.cpp -g -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -o lesson_02 -std=c++11
// LD_LIBRARY_PATH=../bin ./lesson_02

// On os x:
// g++ lesson_02*.cpp -g -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg -o lesson_02 -std=c++11
// DYLD_LIBRARY_PATH=../bin ./lesson_02

// If you have the entire Halide source tree, you can also build it by
// running:
//    make tutorial_lesson_02_input_image
// in a shell with the current directory at the top of the halide
// source tree.

// The only Halide header file you need is Halide.h. It includes all of Halide.
#include "Halide.h"

// Include some support code for loading pngs.
// halide_image_io.h提供了png格式图片的读写函数
#include "halide_image_io.h"
using namespace Halide::Tools;

int main(int argc, char **argv) {

    // This program defines a single-stage imaging pipeline that
    // brightens an image.
    // 该程序定义了一个阶段的图像处理pipeline，将图像对应的像素点放大一定倍数
    // 达到提升图像亮度的目的。

    // First we'll load the input image we wish to brighten.
    // halide_image_io.h提供的图像IO函数，读入要处理的图像数据
    Halide::Buffer<uint8_t> input = load_image("images/rgb.png");

    // See figures/lesson_02_input.jpg for a smaller version.

    // Next we define our Func object that represents our one pipeline
    // stage.
    // 接下来定义Func对象，Func对象表示我们将要进行的图像亮度提升pipeline
    Halide::Func brighter;

    // Our Func will have three arguments, representing the position
    // in the image and the color channel. Halide treats color
    // channels as an extra dimension of the image.
    // 接下来定义操作图像像素的索引，即Var（变量）x（column），y（row），c（channel）
    // x，y为坐标索引，c为颜色通道索引。
    Halide::Var x, y, c;

    // Normally we'd probably write the whole function definition on
    // one line. Here we'll break it apart so we can explain what
    // we're doing at every step.

    // For each pixel of the input image.
    // value表达式表示c通道（x，y）坐标处的像素值
    Halide::Expr value = input(x, y, c);

    // Cast it to a floating point value.
    // 为了进行浮点计算，先将数据类型转换成单精度浮点类型
    value = Halide::cast<float>(value);

    // Multiply it by 1.5 to brighten it. Halide represents real
    // numbers as floats, not doubles, so we stick an 'f' on the end
    // of our constant.
    // 将c通道（x，y）坐标处的像素值放大1.5倍
    value = value * 1.5f;

    // Clamp it to be less than 255, so we don't get overflow when we
    // cast it back to an 8-bit unsigned int.
    // 为了防止数据溢出，将放大后的像素值clip到[0,255]区间，并转换成8位无符号整型
    value = Halide::min(value, 255.0f);

    // Cast it back to an 8-bit unsigned integer.
    value = Halide::cast<uint8_t>(value);

    // Define the function.
    // 定义函数，将亮度提升后的像素值，赋值给函数对象的（x，y，c）点
    brighter(x, y, c) = value;

    // The equivalent one-liner to all of the above is:
    //
    // brighter(x, y, c) = Halide::cast<uint8_t>(min(input(x, y, c) * 1.5f, 255));
    //
    // In the shorter version:
    // - I skipped the cast to float, because multiplying by 1.5f does
    //   that automatically.
    // - I also used an integer constant as the second argument in the
    //   call to min, because it gets cast to float to be compatible
    //   with the first argument.
    // - I left the Halide:: off the call to min. It's unnecessary due
    //   to Koenig lookup.

    // Remember, all we've done so far is build a representation of a
    // Halide program in memory. We haven't actually processed any
    // pixels yet. We haven't even compiled that Halide program yet.
    // 上述所有操作知识在内存中建立Halide程序，告诉Halide怎么去进行算法操作，即对算法进行了定义。
    // 实际上还没有开始进行任何像素的处理。甚至Halide程序还没有进行编译

    // So now we'll realize the Func. The size of the output image
    // should match the size of the input image. If we just wanted to
    // brighten a portion of the input image we could request a
    // smaller size. If we request a larger size Halide will throw an
    // error at runtime telling us we're trying to read out of bounds
    // on the input image.
    // 现在将要实现函数。输出图像的尺寸必须和输入图像的尺寸相匹配。如果我们只想提高输入图像部分区域
    //像素点的亮度，可以指定一个小一点的尺寸。如果需要一个更大尺寸的输出，Halide在运行时会抛出一个
    //错误告诉我们，边界超出输入图像。
    Halide::Buffer<uint8_t> output =
        brighter.realize(input.width(), input.height(), input.channels());

    // Save the output for inspection. It should look like a bright parrot.
    // 写下被处理过的图像
    save_image(output, "brighter.png");

    // See figures/lesson_02_output.jpg for a small version of the output.

    printf("Success!\n");
    return 0;
}
