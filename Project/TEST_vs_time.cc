#include <vector>
#include <algorithm>
#include <functional>
#include <random>
#include <iterator>
#include <iostream>
#include <chrono>
#include <sstream>

#include "ppartquick.hpp"

int main(int argc, char* argv[])
{
  if (4 != argc)
  {
    std::cerr << "usage: " << argv[0] << " <mode> <iterations> <arraysize> \n"
              << "  mode:\n  1: Partitioning\n  2: Quicksort\n"
              << "  3: Quickselect" << std::endl;
    return -1;
  }
  int MODE, RUNS, SIZE;

  if ( !(std::istringstream(argv[1]) >> MODE) || !(MODE > 0) ||
       !(std::istringstream(argv[2]) >> RUNS) || !(RUNS > 0) ||
       !(std::istringstream(argv[3]) >> SIZE) || !(SIZE > 0) )
  {
    std::cerr << "arguments are not a valid positive integer" << std::endl;
    return -1;
  }

  auto clock = std::chrono::high_resolution_clock();
  auto t0 = clock.now();
  auto t1 = clock.now();
  auto time1 = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;
  auto time2 = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;

// TEST ppartition /////////////////////////////////////////////////////////////

  if ( 1 == MODE )
  {
    std::cout << "\nTEST: ppartition ( vectorsize = " << SIZE << ", iterations = " << RUNS << " )\n";
    time1 = 0; time2 = 0;

    for (int i = 0; i < RUNS; ++i) {
      t0 = clock.now();
      std::mt19937 gen(std::random_device{}());
      std::uniform_real_distribution<double> dis(0, 100);
      auto dis1 = std::bind(dis, gen);
      std::vector<double> g(SIZE);
      std::generate(g.begin(), g.end(), dis1);
      t1 = clock.now();
      time1 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;

      t0 = clock.now();
      auto g_it = ppartition(g.begin(), g.end(), [](int i){return i%2 == 0;});
      t1 = clock.now();
      time2 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;
    }
    std::cout << "               ppartition: " << time2 << " s\n";
    std::cout << "                 overhead: " << time1 << " s\n\n";
  }

// TEST pquicksort /////////////////////////////////////////////////////////////

  if ( 2 == MODE )
  {
    std::cout << "\nTEST: pquicksort ( vectorsize = " << SIZE << ", iterations = " << RUNS << " )\n";
    time1 = 0; time2 = 0;

    for(int i = 0; i < RUNS; i++){
      t0 = clock.now();
      std::vector<int> s(SIZE);
      std::mt19937 gen(std::random_device{}());
      std::uniform_int_distribution<> dis(0, 1000);
      auto dis1 = std::bind(dis, gen);
      std::generate(s.begin(), s.end(), dis1);
      t1 = clock.now();
      time1 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;

      t0 = clock.now();
      pquicksort(s.begin(), s.end());
      t1 = clock.now();
      time2 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;
    }

    std::cout << "          pquicksort: " << time2 << " s\n";
    std::cout << "            overhead: " << time1 << " s\n\n";
  }

// TEST pquickselect ///////////////////////////////////////////////////////////

  if ( 3 == MODE )
  {
    std::cout << "\nTEST: pquickselect ( vectorsize = " << SIZE << ", iterations = " << RUNS << " )\n";
    time1 = 0;

    for(int i = 0; i < RUNS; i++){
      t0 = clock.now();
      std::vector<int> t(SIZE);
      std::mt19937 gen(std::random_device{}());
      std::uniform_int_distribution<> dis(0, 100);
      auto dis1 = std::bind(dis, gen);
      std::generate(t.begin(), t.end(), dis1);
      t1 = clock.now();
      time1 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;

      int k = i;

      t0 = clock.now();
      pquickselect(t.begin(), t.begin() + k, t.end());
      t1 = clock.now();
      time2 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;
    }

    std::cout << "               pquickselect: " << time2 << " s\n";
    std::cout << "                   overhead: " << time1 << " s\n\n";
  }

  return 1;
}
