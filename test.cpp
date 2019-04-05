#include "Halide.h"
#include <stdio.h>



//##################################
//#######     Tutorial 2     #######
//##################################
//brighten an image

//using namespace Halide::Tools;
// int main(int argc, char **argv){

//     //first step
// 	Halide::Buffer<uint8_t> input = load_image("images/rgb.png");
// 	Halide::Func brighter;
// 	Halide::Var x,y,c;
// 	Halide::Expr value = input (x,y,c);
// 	brighter(x, y, c) = Halide::cast<uint8_t>(min(input(x, y, c) * 1.5f, 255));

//     //2nd step
// 	Halide::Buffer<uint8_t> output = brighter.realize(input.width(), input.height(), input.channels());
// 	save_image(output,"brighter_bird.png");
// 	return 0;
// }



//##################################
//#######     Tutorial 3     #######
//##################################
// inspecting how llvm work

// using namespace Halide;

// int main(int argc, char **argv){
// 	Func gradient("gradient");
// 	Var x("x"), y("y");
// 	gradient(x, y) = x + y;

// 	Buffer<int> output = gradient.realize(8, 8);

// 	gradient.compile_to_lowered_stmt("gradient.html", {}, HTML);

// 	printf("Passed!");
// 	return 0;

// }




//##################################
//#######     Tutorial 4     #######
//##################################
// familiar with print functions
// using namespace Halide;

// int main(int argc, char **argv){

// 	Func gradient("gradient");

// 	Var x("x"), y("y");

// 	gradient(x, y) = sin(x) + cos(y);

// 	gradient.trace_stores();

// 	gradient.parallel(y);

// 	printf("Evaluating....");

// 	Buffer<int> output = gradient.realize(4, 4);
// }


//##################################
//#######     Tutorial 5     #######
//##################################
// vectorizing, parallel, unrolling, tailing 

using namespace Halide;

int main(int argc, char **argv){

	Func gradient("gradient");

	Var x("x"), y("y");

	gradient(x, y) = x + y;

    gradient.trace_stores();

    Var x_inner, x_outer, y_inner, y_outer;

//    tiling into 4x4 pieces, processing according to order priority
//    gradient.tile(x, y, x_outer, y_outer, x_inner, y_inner, 4, 4);


//    vectorizing, SIMD processing a vector at once
//    gradient.vectorize(x,4);


//    unrolling, in data overlapping case, unroll the data such as 
//	  we only need to compute one.
//    gradient.unroll(x,2);


	Buffer<int> output = gradient.realize(4, 4);

	gradient.print_loop_nest();

	return 0;
}


	//##################################
	//#######     Tutorial 5     #######
	//##################################
	// vectorizing, parallel, unrolling, tailing 