//// Mandelbrot set example
//// Adam Sampson <a.sampson@abertay.ac.uk>
//
////Computing the Mandelbrot set took 0 ms.
////itteration 1 || Computing the Mandelbrot set took 2 ms.
////itteration 2 || Computing the Mandelbrot set took 4 ms.
////itteration 3 || Computing the Mandelbrot set took 16 ms.
////itteration 4 || Computing the Mandelbrot set took 18 ms.
////itteration 5 || Computing the Mandelbrot set took 50 ms.
////itteration 6 || Computing the Mandelbrot set took 75 ms.
////itteration 7 || Computing the Mandelbrot set took 84 ms.
////itteration 8 || Computing the Mandelbrot set took 93 ms.
////itteration 9 || Computing the Mandelbrot set took 118 ms.
////itteration 10 || Computing the Mandelbrot set took 135 ms.
////itteration 11 || Computing the Mandelbrot set took 146 ms.
////itteration 12 || Computing the Mandelbrot set took 160 ms.
//
//
////Whole image 4700 ms
//
//#include <chrono>
//#include <cstdint>
//#include <cstdlib>
//#include <complex>
//#include <fstream>
//#include <iostream>
//#include <thread>
//#include <vector>
//
//// Import things we need from the standard library
//using std::chrono::duration_cast;
//using std::chrono::milliseconds;
//using std::complex;
//using std::cout;
//using std::endl;
//using std::ofstream;
//using std::vector;
//using std::thread;
//
//// Define the alias "the_clock" for the clock type we're going to use.
//typedef std::chrono::steady_clock the_clock;
//
//
//// The size of the image to generate.
//const int WIDTH = 1920;
//const int HEIGHT = 1200;
//
//// The number of times to iterate before we assume that a point isn't in the
//// Mandelbrot set.
//// (You may need to turn this up if you zoom further into the set.)
//const int MAX_ITERATIONS = 500;
//
//// The image data.
//// Each pixel is represented as 0xRRGGBB.
//uint32_t image[HEIGHT][WIDTH];
//
//
//// Write the image to a TGA file with the given name.
//// Format specification: http://www.gamers.org/dEngine/quake3/TGA.txt
//void write_tga(const char* filename)
//{
//	ofstream outfile(filename, ofstream::binary);
//
//	uint8_t header[18] = {
//		0, // no image ID
//		0, // no colour map
//		2, // uncompressed 24-bit image
//		0, 0, 0, 0, 0, // empty colour map specification
//		0, 0, // X origin
//		0, 0, // Y origin
//		WIDTH & 0xFF, (WIDTH >> 8) & 0xFF, // width
//		HEIGHT & 0xFF, (HEIGHT >> 8) & 0xFF, // height
//		24, // bits per pixel
//		0, // image descriptor
//	};
//	outfile.write((const char*)header, 18);
//
//	for (int y = 0; y < HEIGHT; ++y)
//	{
//		for (int x = 0; x < WIDTH; ++x)
//		{
//			uint8_t pixel[3] = {
//				image[y][x] & 0xFF, // blue channel
//				(image[y][x] >> 8) & 0xFF, // green channel
//				(image[y][x] >> 16) & 0xFF, // red channel
//			};
//			outfile.write((const char*)pixel, 3);
//		}
//	}
//
//	outfile.close();
//	if (!outfile)
//	{
//		// An error has occurred at some point since we opened the file.
//		cout << "Error writing to " << filename << endl;
//		exit(1);
//	}
//}
//
//
//// Render the Mandelbrot set into the image array.
//// The parameters specify the region on the complex plane to plot.
//void compute_mandelbrot(double left, double right, double top, double bottom, double roof, double floor, bool wholeImage)
//{
//
//	double floor_ = floor;
//	double roof_ = roof;
//
//	if (wholeImage == true) {
//		floor_ = 0;
//		roof_ = HEIGHT;
//	}
//
//	for (int y = floor_; y < roof_; ++y)
//	{
//		for (int x = 0; x < WIDTH; ++x)
//		{
//			// Work out the point in the complex plane that
//			// corresponds to this pixel in the output image.
//			complex<double> c(left + (x * (right - left) / WIDTH),
//				top + (y * (bottom - top) / HEIGHT));
//
//			// Start off z at (0, 0).
//			complex<double> z(0.0, 0.0);
//
//			// Iterate z = z^2 + c until z moves more than 2 units
//			// away from (0, 0), or we've iterated too many times.
//			int iterations = 0;
//			while (abs(z) < 2.0 && iterations < MAX_ITERATIONS)
//			{
//				z = (z * z) + c;
//
//				++iterations;
//			}
//
//			if (iterations == MAX_ITERATIONS)
//			{
//				// z didn't escape from the circle.
//				// This point is in the Mandelbrot set.
//				image[y][x] = 0x000000; // black
//			}
//			else
//			{
//				// z escaped within less than MAX_ITERATIONS
//				// iterations. This point isn't in the set.
//				image[y][x] = 0xFFFFFF; // white
//			}
//		}
//	}
//}
//
//void InterationTiming() { //split the mandelbrot into 12 segs, set floor and height for each segment, set each segment to a thread
//	int increment = HEIGHT / 12;
//	int floorHeight = 0;
//	int roofHeight = 0;
//
//	vector<thread*> threadVector;
//
//	the_clock::time_point start = the_clock::now();
//	for (int i = 0; i < 12; i++) {
//		roofHeight += increment;
//
//		cout << "thread " << i << " active!\n";
//		thread* tptr = new thread(compute_mandelbrot, -2.0, 1.0, 1.125, -1.125, roofHeight, floorHeight, false);
//		threadVector.push_back(tptr);
//
//		floorHeight += increment;
//	}
//
//	for (thread * t : threadVector) {
//		t->join();
//		cout << "T FINISHED\n";
//	}
//
//	the_clock::time_point end = the_clock::now();
//
//	// Compute the difference between the two times in milliseconds
//	auto time_taken = duration_cast<milliseconds>(end - start).count();
//	cout << "time taken: " << time_taken;
//}
//
//int main(int argc, char* argv[])
//{
//	cout << "Please wait..." << endl;
//
//	// Start timing
//	//the_clock::time_point start = the_clock::now();
//
//	// This shows the whole set.
//	//compute_mandelbrot(-2.0, 1.0, 1.125, -1.125);
//
//	// This zooms in on an interesting bit of detail.
//	//compute_mandelbrot(-0.751085, -0.734975, 0.118378, 0.134488);
//
//	// Stop timing
//	//the_clock::time_point end = the_clock::now();
//	// Compute the difference between the two times in milliseconds
//	//auto time_taken = duration_cast<milliseconds>(end - start).count();
//	//cout << "Computing the Mandelbrot set took " << time_taken << " ms." << endl;
//
//	InterationTiming();
//
//	write_tga("output.tga");
//
//	return 0;
//}