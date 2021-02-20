#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <random>
#include <iterator>
#include <iostream>
#include <chrono>

#include <atomic>

#include "partition.hpp"

template <typename Iter>
std::string a_equal_b(Iter a_first, Iter a_last, Iter b_first, Iter b_last)
{
  Iter b_it = b_first;
  Iter a_it = a_first;
  while (a_it <= a_last)
  {
    if ( *a_it != *b_it) return "failed";
    ++a_it;
    ++b_it;
  }
  return "passed";
}

template <typename Iter>
void printVector(Iter first, Iter last)
{
  for (Iter it = first; it < last; ++it)
    std::cout << *it << (it < last-1 ? ", " : "\n");
}

int main()
{

// check neutralize
/*
    std::cout << "\n neutrazize check: \n";
    std::vector<int> a = {0, 1, 2, 3, 3, 5, 6, 8, 8, 10, 12, 14, 9, 11, 13, 15};
    std::cout << neutralize(a.begin(), a.begin()+9, a.begin()+9, a.end(), [](int i){return i%2 == 0;}) << "\n";
    printVector(a.begin(), a.begin()+9);
    printVector(a.begin()+9, a.end());
*/
//
/*
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
*/

/*
    std::sort(c.begin(), c_it);
    std::sort(c_it, c.end());
    printVector(b.begin(), b.end());
*/
    std::vector<int> c = {1, 3, 3, 5, 4, 6, 6, 7, 8, 10, 12, 14, 14, 12, 16,
                          16, 18, 18, 20, 20, 22, 22, 24, 24, 25, 27, 31, 25, 29, 31};
    //ppartition(d.begin(), d.end(), [](int i){return i%2 == 0;});

    std::vector<int> d = {1, 3, 3, 5, 4, 6, 6, 9, 9, 11, 13, 15, 15, 13, 17,
                          17, 19, 19, 21, 21, 23, 23, 25, 25, 25, 27, 29, 25, 29, 31};
    // ppartition(e.begin(), e.end(), [](int i){return i%2 == 0;});

    std::vector<int> e = {1, 3, 3, 5, 4, 7, 7, 7, 9, 11, 13, 15, 15, 13, 17,
                          17, 19, 19, 21, 21, 23, 23, 25, 25, 25, 27, 31, 25, 29, 31};
    //ppartition(f.begin(), f.end(), [](int i){return i%2 == 0;});

    std::vector<int> f = {2, 4, 4, 6, 4, 6, 6, 9, 12, 12, 14, 16, 16, 14, 18,
                          18, 20, 20, 22, 22, 24, 24, 26, 26, 26, 28, 30, 26, 30, 32};

// ppartition check

    for (int i = 0; i < 1000; ++i) {
      std::mt19937 gen(std::random_device{}());
      std::uniform_int_distribution<> dis(0, 100);
      auto dis1 = std::bind(dis, gen);
      std::vector<int> g(100000);
      std::generate(g.begin(), g.end(), dis1);
      std::vector<int> g2(g);
      auto g_it = std::partition(g.begin(), g.end(), [](int i){return i%2 == 0;});
      auto g2_it = ppartition(g2.begin(), g2.end(), [](int i){return i%2 == 0;});
      std::sort(g.begin(), g_it);
      std::sort(g_it, g.end());
      std::sort(g2.begin(), g2_it);
      std::sort(g2_it, g2.end());
      if (!std::equal(g.begin(), g.end(), g2.begin())){
        std::cout << " BREAK \n";
        break;
      }
      std::cout << i << "\n";
      //std::cout << ((std::equal(g.begin(), g.end(), g2.begin())) ? " passed\n" : " failed\n");
    }

/*
    std::vector<int> g2(g);
    auto g_it = std::partition(g.begin(), g.end(), [](int i){return i%2 == 0;});
    auto g2_it = ppartition(g2.begin(), g2.end(), [](int i){return i%2 == 0;});
    std::sort(g.begin(), g_it);
    std::sort(g_it, g.end());
    std::sort(g2.begin(), g2_it);
    std::sort(g2_it, g2.end());
    std::cout << ((std::equal(g.begin(), g.end(), g2.begin())) ? " passed\n" : " failed\n");
*/
// Test spartition
/*
    std::vector<int> h(20);
    std::generate(h.begin(), h.end(), dis1);
    auto it_h = spartition(h.begin(), h.end(), [](int i){return i < 50;});
    printVector(h.begin(), it_h);
    printVector(it_h, h.end());
*/
// Test quicksort

    std::vector<int> s(100000);
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis(0, 100);
    auto dis1 = std::bind(dis, gen);
    std::generate(s.begin(), s.end(), dis1);
    std::vector<int> s2(s);

    if (!std::equal(s.begin(), s.end(), s2.begin()))
      std::cout << "WARRNING: no equal arrays at the beginning\n";

    auto clock = std::chrono::high_resolution_clock();

    auto t0 = clock.now();
    std::sort(s.begin(), s.end());
    auto t1 = clock.now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;
    std::cout << "std::sort : " << time << " s\n";

    t0 = clock.now();
    pquicksort(s2.begin(), s2.end());
    t1 = clock.now();
    time = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;
    std::cout << "quicksort : " << time << " s \n";

    std::cout << ((std::equal(s.begin(), s.end(), s2.begin())) ? "passed\n" : "failed\n");

    return 0;
}
