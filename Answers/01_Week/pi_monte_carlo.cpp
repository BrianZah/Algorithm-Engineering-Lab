#include <iostream>
#include <omp.h>
#include <random>

using namespace std;

int main()
{
  int n = 100000000; // amount of points to generate
  int counter = 0; // counter for points in the first quarter of a unit circle
  auto start_time = omp_get_wtime(); // omp_get_wtime() is an OpenMP library routine

  // compute n points and test if they lie within the first quadrant of a unit circle
#pragma omp parallel
{
  int num_threads = omp_get_num_threads();
  int thread_id = omp_get_thread_num();

  int m = n / num_threads;
  int cnt_per_thread = 0;

  //random_device seed;
  //default_random_engine re{seed()};
  unsigned long seed = thread_id;
  default_random_engine re{seed};
  uniform_real_distribution<double> zero_to_one{0.0, 1.0};

  for (int i = 0; i < m; ++i)
  {
    auto x = zero_to_one(re); // generate random number between 0.0 and 1.0
    auto y = zero_to_one(re); // generate random number between 0.0 and 1.0
    if (x * x + y * y <= 1.0) // if the point lies in the first quadrant of a unit circle
    {
      ++cnt_per_thread;
    }
  }

  #pragma omp critical
  counter += cnt_per_thread;
}
  auto run_time = omp_get_wtime() - start_time;
  auto pi = 4 * (double(counter) / n);

  cout << "pi: " << pi << endl;
  cout << "run_time: " << run_time << " s" << endl;
  cout << "n: " << n << endl; }
