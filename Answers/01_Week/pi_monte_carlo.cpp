#include <iostream>
#include <omp.h>
#include <random>

using namespace std;

int main() {
  random_device seed;
  default_random_engine re{seed()};
  uniform_real_distribution<double> zero_to_one{0.0, 1.0};

  int n = 100000000; // amount of points to generate
  int counter = 0; // counter for points in the first quarter of a unit circle
  auto start_time = omp_get_wtime(); // omp_get_wtime() is an OpenMP library routine

  // compute n points and test if they lie within the first quadrant of a unit circle
  for (int i = 0; i < n; ++i) {
    auto x = zero_to_one(re); // generate random number between 0.0 and 1.0
    auto y = zero_to_one(re); // generate random number between 0.0 and 1.0
    if (x * x + y * y <= 1.0) { // if the point lies in the first quadrant of a unit circle
      ++counter;
    }
  }

  auto run_time = omp_get_wtime() - start_time;
  auto pi = 4 * (double(counter) / n);

  cout << "pi: " << pi << endl;
  cout << "run_time: " << run_time << " s" << endl;
  cout << "n: " << n << endl; }
