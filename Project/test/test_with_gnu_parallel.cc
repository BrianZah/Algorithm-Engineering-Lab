#include <omp.h>

#include <vector>
#include <algorithm>
#include <parallel/algorithm>
#include <functional>
#include <random>
#include <iterator>
#include <iostream>
#include <chrono>
#include <sstream>

#include "ppartquick.hpp"

template <typename Iter>
void printVector( Iter first, Iter last )
{
  for( Iter it = first; it < last; ++it )
    std::cout << *it << (it < last-1 ? ", " : "\n");
}

template <typename Iter>
void generateRandomIntVector( Iter first, Iter last )
{
  long SIZE = last - first;
  int THREADS = omp_get_max_threads();
  int work_share = SIZE / THREADS;

#pragma omp parallel for
  for( int i = 0; i < THREADS; ++i )
  {
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis( INT32_MIN, INT32_MAX );
    auto dis1 = std::bind( dis, gen );

    if( omp_get_thread_num() == THREADS-1)
      std::generate( first + i*work_share , last, dis1 );
    else
      std::generate( first + i*work_share , first + (i+1)*work_share, dis1 );
  }
}


int main( int argc, char* argv[] )
{
  if(4 != argc)
  {
    std::cerr << "usage: " << argv[0] << " <mode> <iterations> <arraysize> \n"
              << "  mode:\n  1: Partitioning\n  2: Quicksort\n"
              << "  3: Quickselect\n  4: 1 & 2\n  5: 1 & 3\n  6: 2 & 3\n"
              << "  7: 1 & 2 & 3" << std::endl;
    return -1;
  }
  int MODE, RUNS;
  long SIZE;

  if( !(std::istringstream( argv[1]) >> MODE ) || !(MODE > 0) ||
      !(std::istringstream( argv[2]) >> RUNS ) || !(RUNS > 0) ||
      !(std::istringstream( argv[3]) >> SIZE ) || !(SIZE > 0) )
  {
    std::cerr << "arguments are not a valid positive integer" << std::endl;
    return -1;
  }

  auto clock = std::chrono::high_resolution_clock();
  auto t0 = clock.now();
  auto t1 = clock.now();
  auto time0 = std::chrono::duration_cast<std::chrono::nanoseconds>( t1 - t0 ).count()/1.0E9;
  auto time1 = std::chrono::duration_cast<std::chrono::nanoseconds>( t1 - t0 ).count()/1.0E9;
  auto time2 = std::chrono::duration_cast<std::chrono::nanoseconds>( t1 - t0 ).count()/1.0E9;

// TEST ppartition /////////////////////////////////////////////////////////////
  if ( 1 == MODE || 4 == MODE || 5 == MODE || 7 == MODE )
  {
    std::cout << "\nTEST: ppartition ( vectorsize = " << SIZE << ", iterations = " << RUNS << " )\n";
    time0 = 0; time1 = 0; time2 = 0;

    for( int i = 0; i < RUNS; ++i )
    {
      std::vector<int> g( SIZE );
      generateRandomIntVector( g.begin(), g.end() );
      std::vector<int> g2( g );
      std::vector<int> g3( g );

      if( !std::equal( g.begin(), g.end(), g2.begin() ) )
        std::cout << "WARRNING: no equal arrays at the beginning\n";

      t0 = clock.now();
      auto g_it = __gnu_parallel::partition( g.begin(), g.end(), [](int i){return i%2 == 0;} );
      t1 = clock.now();
      time0 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;

      t0 = clock.now();
      auto g2_it = ppartition( g2.begin(), g2.end(), [](int i){return i%2 == 0;} );
      t1 = clock.now();
      time1 += std::chrono::duration_cast<std::chrono::nanoseconds>( t1 - t0 ).count()/1.0E9;

      t0 = clock.now();
      auto g3_it = std::partition( g3.begin(), g3.end(), [](int i){return i%2 == 0;} );
      t1 = clock.now();
      time2 += std::chrono::duration_cast<std::chrono::nanoseconds>( t1 - t0 ).count()/1.0E9;

      __gnu_parallel::sort( g.begin(), g_it );
      __gnu_parallel::sort( g_it, g.end() );
      __gnu_parallel::sort( g2.begin(), g2_it );
      __gnu_parallel::sort( g2_it, g2.end() );

      if( !std::equal( g.begin(), g.end(), g2.begin() ) )
      {
        std::cout << " FAILED ( turn: " << i << " )\n";
        break;
      }
    }
    std::cout << "__gnu_parallel::partition: " << time0 << " s\n";
    std::cout << "               ppartition: " << time1 << " s\n";
    std::cout << "           std::partition: " << time2 << " s\n\n";
  }

// TEST pquicksort /////////////////////////////////////////////////////////////
  if( 2 == MODE || 4 == MODE || 6 == MODE || 7 == MODE )
  {
    std::cout << "\nTEST: pquicksort ( vectorsize = " << SIZE << ", iterations = " << RUNS << " )\n";
    time0 = 0; time1 = 0; time2 = 0;

    for( int i = 0; i < RUNS; i++ )
    {
      std::vector<int> s( SIZE );
      generateRandomIntVector( s.begin(), s.end() );
      std::vector<int> s2( s );
      std::vector<int> s3( s );

      if( !std::equal( s.begin(), s.end(), s2.begin() ) ||
          !std::equal( s.begin(), s.end(), s3.begin() ) )
        std::cout << "WARRNING: no equal arrays at the beginning\n";

      t0 = clock.now();
      __gnu_parallel::sort( s.begin(), s.end() );
      t1 = clock.now();
      time0 += std::chrono::duration_cast<std::chrono::nanoseconds>( t1 - t0 ).count()/1.0E9;

      t0 = clock.now();
      pquicksort( s2.begin(), s2.end() );
      t1 = clock.now();
      time1 += std::chrono::duration_cast<std::chrono::nanoseconds>( t1 - t0 ).count()/1.0E9;

      t0 = clock.now();
      //pquicksort_dual_pivot( s3.begin(), s3.end() );
      std::sort( s3.begin(), s3.end() );
      t1 = clock.now();
      time2 += std::chrono::duration_cast<std::chrono::nanoseconds>( t1 - t0 ).count()/1.0E9;

      if( !std::equal(s.begin(), s.end(), s2.begin()) )
      {
        std::cout << " FAILED ( turn: " << i << " )\n";
        break;
      }

    }
    std::cout << " __gnu_parallel::sort: " << time0 << " s\n";
    std::cout << "           pquicksort: " << time1 << " s\n";
    std::cout << "            std::sort: " << time2 << " s\n\n";
  }
// TEST pquickselect ///////////////////////////////////////////////////////////
  if( 3 == MODE || 5 == MODE || 6 == MODE || 7 == MODE )
  {
    std::cout << "\nTEST: pquickselect ( vectorsize = " << SIZE << ", iterations = " << RUNS << " )\n";
    time0 = 0; time1 = 0; time2 = 0;

    for( int i = 0; i < RUNS; i++ )
    {
      std::vector<int> t( SIZE );
      generateRandomIntVector( t.begin(), t.end() );
      std::vector<int> t2( t );
      std::vector<int> t3( t );

      int k = i * ( SIZE / RUNS );

      if( !std::equal( t.begin(), t.end(), t2.begin() ) ||
          !std::equal( t.begin(), t.end(), t3.begin() ) )
        std::cout << "WARRNING: no equal arrays at the beginning\n";

      t0 = clock.now();
      __gnu_parallel::nth_element( t.begin(), t.begin() + k, t.end() );
      t1 = clock.now();
      time0 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()/1.0E9;

      t0 = clock.now();
      pquickselect( t2.begin(), t2.begin() + k, t2.end() );
      t1 = clock.now();
      time1 += std::chrono::duration_cast<std::chrono::nanoseconds>( t1 - t0 ).count()/1.0E9;

      t0 = clock.now();
      std::nth_element( t3.begin(), t3.begin() + k, t3.end() );
      t1 = clock.now();
      time2 += std::chrono::duration_cast<std::chrono::nanoseconds>( t1 - t0 ).count()/1.0E9;

      if( ( *(t.begin() + k) != *(t2.begin() + k) ) ||
          ( *(t.begin() + k) != *(t3.begin() + k) ) )
      {
        std::cout << " FAILED ( turn: " << i << " )\n";
        break;
      }

    }
    std::cout << "__gnu_parallel::nth_element: " << time0 << " s\n";
    std::cout << "               pquickselect: " << time1 << " s\n";
    std::cout << "           std::nth_element: " << time2 << " s\n\n";
  }
  return 0;
}
