#include <vector>
#include <algorithm>
#include <parallel/algorithm>
#include <functional>
#include <random>
#include <iterator>
#include <iostream>
#include <chrono>

#include "partition.hpp"

template <typename Iter>
void printVector(Iter first, Iter last)
{
  for (Iter it = first; it < last; ++it)
    std::cout << *it << (it < last-1 ? ", " : "\n");
}

int main()
{
  auto clock = std::chrono::high_resolution_clock();
  auto t0 = clock.now();
  auto t1 = clock.now();
  auto time0 = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;
  auto time1 = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;

  int RUNS, SIZE;

// TEST ppartition /////////////////////////////////////////////////////////////
/*
  RUNS = 100; SIZE = 1000000;
  std::cout << "\nTEST: ppartition ( vectorsize = " << SIZE << ", iterations = " << RUNS << " )\n";
  time0 = 0; time1 = 0;

  for (int i = 0; i < RUNS; ++i) {
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> dis(0, 100);
    auto dis1 = std::bind(dis, gen);
    std::vector<float> g(SIZE);
    std::generate(g.begin(), g.end(), dis1);
    std::vector<float> g2(g);

    t0 = clock.now();
    auto g_it = __gnu_parallel::partition(g.begin(), g.end(), [](int i){return i%2 == 0;});
    t1 = clock.now();
    time0 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;

    t0 = clock.now();
    auto g2_it = ppartition(g2.begin(), g2.end(), [](int i){return i%2 == 0;});
    t1 = clock.now();
    time1 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;


    pquicksort(g.begin(), g_it);
    pquicksort(g_it, g.end());
    pquicksort(g2.begin(), g2_it);
    pquicksort(g2_it, g2.end());

    if ( !std::equal(g.begin(), g.end(), g2.begin()) ) {
      std::cout << " FAILED ( turn: " << i << " )\n";
      break;
    }
  }
  std::cout << "__gnu_parallel::partition: " << time0 << " s\n";
  std::cout << "               ppartition: " << time1 << " s\n\n";
*/

// TEST pquicksort /////////////////////////////////////////////////////////////

  RUNS = 1; SIZE = 1000000;
  std::cout << "\nTEST: pquicksort ( vectorsize = " << SIZE << ", iterations = " << RUNS << " )\n";
  time0 = 0; time1 = 0;

  for(int i = 0; i < RUNS; i++){
    std::vector<int> s(SIZE);
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis(0, 1000);
    auto dis1 = std::bind(dis, gen);
    std::generate(s.begin(), s.end(), dis1);
    std::vector<int> s2(s);

    if (!std::equal(s.begin(), s.end(), s2.begin()))
      std::cout << "WARRNING: no equal arrays at the beginning\n";

    t0 = clock.now();
    __gnu_parallel::sort(s.begin(), s.end());
    t1 = clock.now();
    time0 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;

    t0 = clock.now();
    pquicksort(s2.begin(), s2.end());
    t1 = clock.now();
    time1 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;

    if ( !std::equal(s.begin(), s.end(), s2.begin()) ) {
      std::cout << " FAILED ( turn: " << i << " )\n";
      break;
    }
  }
  std::cout << "__gnu_parallel::sort: " << time0 << " s\n";
  std::cout << "          pquicksort: " << time1 << " s\n\n";

// TEST pquickselect ///////////////////////////////////////////////////////////
/*
  RUNS = 100; SIZE = 1000000;
  std::cout << "\nTEST: pquickselect ( vectorsize = " << SIZE << ", iterations = " << RUNS << " )\n";
  time0 = 0; time1 = 0;

  for(int i = 0; i < RUNS; i++){
    std::vector<int> t(SIZE);
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis(0, 100);
    auto dis1 = std::bind(dis, gen);
    std::generate(t.begin(), t.end(), dis1);
    std::vector<int> t2(t);

    int k = i;
    //std::cout << "==========================================================\n";
    //std::cout << *(t.begin() + k) << " " << *(t2.begin() + k) << " " << k << "\n";

    if (!std::equal(t.begin(), t.end(), t2.begin()))
      std::cout << "WARRNING: no equal arrays at the beginning\n";

    t0 = clock.now();
    __gnu_parallel::nth_element(t.begin(), t.begin() + k, t.end());
    t1 = clock.now();
    time0 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;


    t0 = clock.now();
    pquickselect(t2.begin(), t2.begin() + k, t2.end());
    t1 = clock.now();
    time1 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;

    //std::cout << *(t.begin() + k) << " " << *(t2.begin() + k) << " " << k << "\n";
    if ((*(t.begin() + k) != *(t2.begin() + k))){
      std::cout << " FAILED ( turn: " << i << " )\n";
      break;
    }

  }
  std::cout << "__gnu_parallel::nth_element: " << time0 << " s\n";
  std::cout << "               pquickselect: " << time1 << " s\n\n";
*/


/* SPECIAL CASES
  std::cout << "\n neutrazize check: \n";
  std::vector<int> a = {0, 1, 2, 3, 3, 5, 6, 8, 8, 10, 12, 14, 9, 11, 13, 15};
  std::cout << neutralize(a.begin(), a.begin()+9, a.begin()+9, a.end(), [](int i){return i%2 == 0;}) << "\n";
  printVector(a.begin(), a.begin()+9);
  printVector(a.begin()+9, a.end());

  std::cout << "\n ppartition check: \n";
  std::vector<int> b = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  std::vector<int> b2(b);
  auto b_it = std::partition(b.begin(), b.end(), [](int i){return i%2 == 0;});
  auto b2_it = ppartition(b2.begin(), b2.end(), [](int i){return i%2 == 0;});
  std::sort(b.begin(), b_it);
  std::sort(b_it, b.end());
  std::sort(b2.begin(), b2_it);
  std::sort(b2_it, b2.end());
  std::cout << ((std::equal(b.begin(), b.end(), b2.begin())) ? " passed\n" : " failed\n");

  std::vector<int> c = {1, 3, 3, 5, 4, 6, 6, 7, 8, 10, 12, 14, 14, 12, 16,
                        16, 18, 18, 20, 20, 22, 22, 24, 24, 25, 27, 31, 25, 29, 31};
  ppartition(d.begin(), d.end(), [](int i){return i%2 == 0;});

  std::vector<int> d = {1, 3, 3, 5, 4, 6, 6, 9, 9, 11, 13, 15, 15, 13, 17,
                        17, 19, 19, 21, 21, 23, 23, 25, 25, 25, 27, 29, 25, 29, 31};
  ppartition(e.begin(), e.end(), [](int i){return i%2 == 0;});

  std::vector<int> e = {1, 3, 3, 5, 4, 7, 7, 7, 9, 11, 13, 15, 15, 13, 17,
                        17, 19, 19, 21, 21, 23, 23, 25, 25, 25, 27, 31, 25, 29, 31};
  ppartition(f.begin(), f.end(), [](int i){return i%2 == 0;});

  std::vector<int> f = {2, 4, 4, 6, 4, 6, 6, 9, 12, 12, 14, 16, 16, 14, 18,
                        18, 20, 20, 22, 22, 24, 24, 26, 26, 26, 28, 30, 26, 30, 32};

  std::vector<int> g2(g);
  auto g_it = std::partition(g.begin(), g.end(), [](int i){return i%2 == 0;});
  auto g2_it = ppartition(g2.begin(), g2.end(), [](int i){return i%2 == 0;});
  std::sort(g.begin(), g_it);
  std::sort(g_it, g.end());
  std::sort(g2.begin(), g2_it);
  std::sort(g2_it, g2.end());
  std::cout << ((std::equal(g.begin(), g.end(), g2.begin())) ? " passed\n" : " failed\n");

// Test spartition
  std::vector<int> h(20);
  std::generate(h.begin(), h.end(), dis1);
  auto it_h = spartition(h.begin(), h.end(), [](int i){return i < 50;});
  printVector(h.begin(), it_h);
  printVector(it_h, h.end());
*/

  return 0;
}
