#include <iomanip>
#include <iostream>
#include <omp.h>

using namespace std;

int main() {
  int num_steps = 100000000; // amount of rectangles
  double width = 1.0 / double(num_steps); // width of a rectangle
  double sum = 0.0; // for summing up all heights of rectangles
  double start_time = omp_get_wtime(); // wall clock time in seconds
  omp_set_num_threads(4); // setting the desired number of threads in the parallel region
#pragma omp parallel // parallel region starts here
  { // idea for parallelisation: split up loop iterations between threads
    double sum_local = 0.0; // for summing up heights locally
#pragma omp for
    for (int i = 0; i < num_steps; ++i) {
      double x = (i + 0.5) * width; // midpoint
      sum_local = sum_local + (1.0 / (1.0 + x * x)); // add new height
    }
#pragma omp atomic // only one thread performs the update of the sum
    sum += sum_local;
  } // parallel region ends here
  double pi = sum * 4 * width; // compute pi
  double run_time = omp_get_wtime() - start_time;
  cout << "pi with " << num_steps << " steps is " << setprecision(17) << pi
       << " in " << setprecision(6) << run_time << " seconds\n";
}
