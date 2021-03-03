#ifndef PPARTQUICK_HPP
#define PPARTQUICK_HPP

#include <omp.h>
#include <atomic>
// defines block size
// Please adopt this variable when using the library
// it depands on used data type and the sysemts L1-cache
// two blocks should fit in L1-Cache:
// Size(DType) * B * 2 = L1-Cache
// Using simultaneous multi-threading / hyperthreading:
// Size(DType) * B * 2 * 2 = L1-Cache
#define B 2096

// receives two blocks and obtains one left-side or one right-side block or both
// returns 1 for a left-side, 2 for a right.side block, and 3 for both
template< class FwdIt, class Predicate >
inline constexpr int neutralize( const FwdIt left_first, const FwdIt left_last,
                                 const FwdIt right_first, const FwdIt right_last,
                                 const Predicate pred )
{
  auto left_it = left_first;
  auto right_it = right_first;
  while( left_it <= left_last && right_it <= right_last)
  {
    for( ; left_it < left_last; left_it++ )
      if( !pred(*left_it) ) break;

    for( ; right_it < right_last; right_it++ )
      if( pred(*right_it) ) break;

    if( (left_it == left_last) || (right_it == right_last) ) break;

    auto tmp = *left_it;
    *left_it = *right_it;
    *right_it = tmp;

    ++left_it; ++right_it;
  }

  if( (left_it == left_last) && (right_it == right_last) ) return 2;
  if( left_it == left_last ) return 0;
  return 1;
}

// extracts block from the left side of the array
template< class FwdIt >
inline void getLeftBlock( const FwdIt first, const FwdIt last,
                          FwdIt &left_first, FwdIt &left_last,
                          std::atomic<int> &numRemainingBlocks, std::atomic<int> &i )
{
  if( 0 < std::atomic_fetch_sub( &numRemainingBlocks, 1 ) )
  {
    auto ii = std::atomic_fetch_add( &i, 1 );
    left_first = first + ii*B;
    left_last = first + ii*B + B;
  }
  else
  {
    left_first = last;
    left_last = last;
  }
}

// extracts block from the right side of the array
template< class FwdIt >
inline void getRightBlock( const FwdIt first, const FwdIt last,
                           FwdIt &right_first, FwdIt &right_last,
                           std::atomic<int> &numRemainingBlocks, std::atomic<int> &j,
                           const long N )
{
  if( 0 < std::atomic_fetch_sub( &numRemainingBlocks, 1 ) )
  {
    auto jj = std::atomic_fetch_add( &j, 1 );
    right_first = last - N%B - jj*B;
    right_last = last - N%B - jj*B + B;
  }
  else
  {
    right_first = last;
    right_last = last;
  }
}

// all processors partition the array blockwise
// first, last = array borders
// num = number of threads
// LN, RN = partitioned left/right-side elements
// p = number of remaining Blocks after parallel phase
// remainingBlocks = remembers first element-index of all remaining blocks
template< class FwdIt, class Predicate >
inline void parallel_phase( const FwdIt first, const FwdIt last,
                            const Predicate pred, const int num,
                            long &LN, long &RN, int &p, long *remainingBlocks )
{
  const long N = last - first;
  LN = 0;
  RN = 0;
  p = 0;
  // variables for assigning threads
  std::atomic<int> numRemainingBlocks( (N-1) / B + 1 );
  std::atomic<int> i( 0 );
  std::atomic<int> j( 0 );
  if (N%B == 0) j++;

  for( int tid = 0; tid < num; ++tid )
  {
#pragma omp task firstprivate( tid ) shared( LN, RN, p, i, j, numRemainingBlocks ) if( num > 1 )
{
    FwdIt left_first, left_last, right_first, right_last;
    long LN_ = 0;
    long RN_ = 0;
    int p_ = 0;

    getLeftBlock( first, last, left_first, left_last, numRemainingBlocks, i );
    getRightBlock( first, last, right_first, right_last, numRemainingBlocks, j, N );
    if( right_last > last ) right_last = last;

    while( (left_first != last) && (right_first != last) )
    {
      auto result = neutralize( left_first, left_last, right_first, right_last, pred );
      if( result%2 == 0 ) // left-side block was obtained
      {
        LN_ += left_last-left_first;
        getLeftBlock( first, last, left_first, left_last, numRemainingBlocks, i );
      }
      if( result > 0 ) // right-side block was obtained
      {
        RN_ += right_last - right_first;
        getRightBlock( first, last, right_first, right_last, numRemainingBlocks, j, N );
      }
    }
    remainingBlocks[tid] = last - first;
    if( left_first != last ) // remember left block if not finished
    {
      remainingBlocks[tid] = left_first - first;
      p_++;
    }
    else if( right_first != last ) // remember right block if not finished
    {
      remainingBlocks[tid] = right_first - first;
      p_++;
    }
#pragma omp atomic
    RN += RN_;
#pragma omp atomic
    LN += LN_;
#pragma omp atomic
    p += p_;
}// end omp task
  }
#pragma omp taskwait
}

// launches a parallel region if not has been started already
// invokes the actual parallel phase
template< class FwdIt, class Predicate >
inline void parallel_phase_run( const bool omp_parallel_active,
                                const FwdIt first, const FwdIt last,
                                const Predicate pred, const int num,
                                long &LN, long &RN,
                                int &p, long *remainingBlocks )
{
  if( omp_parallel_active )
  {
    parallel_phase( first, last, pred, num, LN, RN, p, remainingBlocks );
  }
  else
  {
#pragma omp parallel
#pragma omp single
    parallel_phase( first, last, pred, num, LN, RN, p, remainingBlocks );
  }
}

// is faster than quicksort for small arrays
template< class FwdIt >
inline void insertion_sort( FwdIt first, FwdIt last )
{
  int i = 1;
  while( i < (last - first) )
  {
    const auto x = *(first + i);
    int j = i-1;
    while( ( j >= 0 ) && ( *(first + j) > x ) )
    {
      *(first + j + 1) = *(first + j);
      --j;
    }
    *(first + j + 1) = x;
    ++i;
  }
}

// one processor partitions the array blockwise
// first, last = array borders
// num = number of threads in previous parallel_phase
// p = number of remaining blocks after parallel phase
// LN, RN = partitioned left/right-side elements
// left, right = remember state of processed remainingBlocks
// remainingBlocks = remembers first element-index of all remaining blocks
template< class FwdIt, class Predicate >
inline void sequ_neutralization( const FwdIt first, const FwdIt last,
                                 const Predicate pred, const int num, const int p,
                                 long &LN, long &RN, int &left, int &right,
                                 long *remainingBlocks )
{
  // sorts remaining blocks (smallest element-index first)
  insertion_sort( remainingBlocks, remainingBlocks+num );

  const long N = last - first;
  left = 0;
  right = p-1;

  auto left_first = first + remainingBlocks[left];
  auto left_last = left_first + B;
  auto right_first = first + remainingBlocks[right];
  auto right_last = right_first + B;
  if (right_last > last) right_last = last;

  while( left < right )
  {
    auto result = neutralize(left_first, left_last, right_first, right_last, pred);
    if( result%2 == 0 ) // left-side block was obtained
    {
      if( left_first <= first + LN )
      {
        LN += B;
        remainingBlocks[left] = last - first;
      }
      left++;
      left_first = first + remainingBlocks[left];
      left_last = left_first + B;
    }
    if( result > 0 ) // right-side block was obtained
    {
      if ( right_first >= last - RN - B )
      {
        RN += right_last - right_first;
        remainingBlocks[right] = last - first;
      }
      right--;
      right_first = first + remainingBlocks[right];
      right_last = right_first + B;
    }
  }
}

// swapps two blocks
template< class FwdIt >
inline void swapBlocks( const FwdIt left_first, const FwdIt left_last,
                        const FwdIt right_first, const FwdIt right_last )
{
  if( right_first != left_first )
  {
    auto tmp = *left_first;
    auto left_it = left_first;
    auto right_it = right_first;
    while( (left_it < left_last) && (right_it < right_last) )
    {
      tmp = *left_it;
      *left_it = *right_it;
      *right_it = tmp;
      left_it++;
      right_it++;
    }
  }
}

// one processor swapps blocks into the inteded position
// first, last = array borders
// p = number of remaining blocks after parallel phase
// LN, RN = partitioned left/right-side elements
// left, right = remember state of processed remainingBlocks
// remainingBlocks = remembers first element-index of all remaining blocks
template< class FwdIt >
inline void sequ_swapping( const FwdIt first, const FwdIt last,
                           const int p, const int left, const int right,
                           long &LN, long &RN, long *remainingBlocks )
{
  const long N = last - first;
  FwdIt left_first, left_last, right_first, right_last;
  // right blocks are swapped into intended position
  for( int i = right+1; i < p; i++ )
  {
    if( remainingBlocks[i] != last - first )
    {
      left_first = last - RN - B;
      left_last = left_first + B;
      right_first = first + remainingBlocks[i];
      right_last = right_first + B;

      swapBlocks( left_first, left_last, right_first, right_last );
      RN += B;

      for( int k = p-1; k > i; k-- )
      {
        if( remainingBlocks[k] == N - RN - B )
        {
          remainingBlocks[k] = last - first;
          RN += B;
        }
      }
    }
  }
  // left blocks are swapped into intended position
  for( int i = left-1; i >= 0 ; i-- )
  {
    if( remainingBlocks[i] != last - first )
    {
      right_first = first + LN;
      right_last = right_first + B;
      left_first = first + remainingBlocks[i];
      left_last = left_first + B;

      swapBlocks( left_first, left_last, right_first, right_last );
      LN += B;

      for( int k = 0; k < i; k++ )
      {
        if( remainingBlocks[k] == LN )
        {
          remainingBlocks[k] = last - first;
          LN += B;
        }
      }
    }
  }
  // not neutralzed block is swapped into intended position
  if( left == right )
  {
    right_first = first + LN;
    right_last = right_first + B;
    left_first = first + remainingBlocks[right];
    left_last = left_first + B;
    if( left_last > last ) left_last = last;

    swapBlocks( left_first, left_last, right_first, right_last );
  }
}

// partitions an array single-threaded
template< class FwdIt, class Predicate >
constexpr FwdIt spartition( const FwdIt first, const FwdIt last,
                            const Predicate pred )
{
  int split = 0;
  // finds splitting point
  for( auto iter = first; iter < last; ++iter )
  {
    if( pred(*iter) ) ++split;
  }
  // partitions array and returns first element of right-side group
  neutralize( first, first+split, first+split, last, pred );
  return first+split;
}

// combines previous parts to a parallel partioner
template< class FwdIt, class Predicate >
constexpr FwdIt ppartition( const FwdIt first, const FwdIt last,
                            const Predicate pred,
                            const int num = omp_get_max_threads(),
                            const bool omp_parallel_active = false )
{
  // partitioned left-side / right-side elements
  long LN, RN;
  // number of unsorted blocks after parallel_phase
  int p;
  // counter for remaining left/right-sided blocks
  int left, right;
  // here, every processors inserts its remaining block after the parallel_phase
  long remainingBlocks[num];
  // all processors partition the array blockwise
  parallel_phase_run( omp_parallel_active, first, last, pred, num,
                      LN, RN, p, remainingBlocks );
  // the remaining blocks are partitioned sequentially
  sequ_neutralization( first, last, pred, num, p,
                       LN, RN, left, right, remainingBlocks );
  // the sequentially-partitioned blocks are ordered
  sequ_swapping( first, last, p, left, right, LN, RN, remainingBlocks );
  // the last remaining block is partitioned and
  // the first element of the right-side group is returned
  return spartition( first+LN, last-RN, pred );
}

template< class Elem >
Elem medianOfThree( const Elem a, const Elem b, const Elem c )
{
    if( (a > b) xor (a > c) )
        return a;
    else if( (b < a) xor (b < c) )
        return b;
    else
        return c;
}

// standard quicksort, per default single threaded
// launch with pquicksort to run in parallel
// num = number of threads, only for intern use
template< class FwdIt, class Compare = std::less<> >
void quicksort( const FwdIt first, const FwdIt last,
                const Compare cmp = Compare{},
                const int num = 1 )
{
  const long distance = std::distance( first, last );
  // insertionsort is faster for small arrays
  if( distance <= 32 )
  {
    insertion_sort( first, last );
    return;
  }
  // median of three as pivot is more robust for natrual distributions
  const auto pivot = medianOfThree( *first, *std::prev( last, 1 ),
                                    *std::next( first, distance/2 ) );

  // two partitionings to avoid getting stuck
  const auto cmp1 = [=]( const auto &elem ){ return cmp( elem, pivot ); };
  const auto cmp2 = [=]( const auto &elem ){ return !cmp( pivot, elem ); };
  FwdIt middle1, middle2;

  // ppartitioning is more efficient for arrays not fitting in cache
  // spartitioning has less overhead once the array fitts in cache
  if( distance >= 2*B )
  {
    middle1 = ppartition( first, last, cmp1, num, true );
    middle2 = ppartition( middle1, last, cmp2, num, true );
  }
  else
  {
    middle1 = spartition( first, last, cmp1 );
    middle2 = spartition( middle1, last, cmp2 );
  }

  // ONLY necessary for parallel quicksort (see below)
  // distributs cores according to remaining work
  int new_num1 = num;
  int new_num2 = num;
  const long distance1 = std::distance( first, middle1 );
  const long distance2 = std::distance( middle2, last );
  if( num > 1 )
  {
    const float share = 1.0 * distance1 / ( distance1 + distance2 );
    new_num1 = ( (int)(share * num) < 1 ) ? 1 : share * num;
    new_num2 = ( (num - new_num1) < 1 ) ? 1 : num - new_num1;
  }
  // recursive quicksort calls
  // pragmas are ONLY considered when invoked from parallel quicksort
  // if arraysize over 10000, start new tasks
#pragma omp task if( distance1 > 10000 )
  quicksort( first, middle1, cmp, new_num1 );
#pragma omp task if( distance2 > 10000 )
  quicksort( middle2, last, cmp, new_num2 );
// omp taskwait is necessary for the icpc compiler
// please comment out for max performance with the g++ compiler
#pragma omp taskwait
}

// parallel quicksort starter
template< class FwdIt, class Compare = std::less<> >
void pquicksort( const FwdIt first, const FwdIt last,
                 const Compare cmp = Compare{} )
{
#pragma omp parallel
#pragma omp single
  quicksort( first, last, cmp, omp_get_max_threads() );
}

// dual pivot quicksort, per default single threaded
// launch with pquicksort to run in parallel
// num = number of threads, only for intern use
template< class FwdIt, class Compare = std::less<> >
void quicksort_dual_pivot( const FwdIt first, const FwdIt last,
                           const Compare cmp = Compare{},
                           const int num = 1 )
{
  const int distance = std::distance( first, last );
  // insertionsort is faster for small arrays
  if( distance <= 32 )
  {
    insertion_sort(first, last);
    return;
  }
  // median of three as pivot is more robust for natrual distributions
  auto pivot1 = medianOfThree( *first, *std::prev( last, 1 ),
                               *std::next( first, distance/2 ) );
  auto pivot2 = medianOfThree( *std::next( first, distance/2 ),
                               *std::next( first, 3*distance/4 ),
                               *std::prev( last, 1 ) );
  // swap pivots if second is smaller than first one
  if ( pivot1 > pivot2 ){
    auto tmp = pivot1;
    pivot1 = pivot2;
    pivot2 = tmp;
  }
  // four partitionings to avoid getting stuck
  const auto cmp11 = [=]( const auto &elem ){ return cmp( elem, pivot1 ); };
  const auto cmp12 = [=]( const auto &elem ){ return !cmp( pivot1, elem ); };
  const auto cmp21 = [=]( const auto &elem ){ return cmp( elem, pivot2 ); };
  const auto cmp22 = [=]( const auto &elem ){ return !cmp( pivot2, elem ); };
  FwdIt middle11, middle12, middle21, middle22;

  // ppartitioning is more efficient for arrays not fitting in cache
  // spartitioning has less overhead once the array fitts in cache
  if( distance >= 2*B )
  {
    middle11 = ppartition( first, last, cmp11, num, true );
    middle12 = ppartition( middle11, last, cmp12, num, true );
    middle21 = ppartition( middle12, last, cmp21, num, true );
    middle22 = ppartition( middle21, last, cmp22, num, true );
  }
  else
  {
    middle11 = spartition( first, last, cmp11 );
    middle12 = spartition( middle11, last, cmp12 );
    middle21 = spartition( middle12, last, cmp21 );
    middle22 = spartition( middle21, last, cmp22 );
  }
  // ONLY necessary for parallel quicksort (see below)
  // distributs cores according to remaining work
  int new_num1 = num;
  int new_num2 = num;
  int new_num3 = num;
  const long distance1 = std::distance( first, middle11 );
  const long distance2 = std::distance( middle12, middle21 );
  const long distance3 = std::distance( middle22, last );

  if ( num > 1 ) {
    const long elems = distance1 + distance2 + distance3;
    const float share1 = 1.0 * distance1 / elems;
    const float share2 = 1.0 * distance2 / elems;
    new_num1 = ((int)(share1 * num) < 1) ? 1 : share1 * num;
    new_num2 = ((int)(share2 * num) < 1) ? 1 : share2 * num;
    new_num3 = ((num-new_num1-new_num2) < 1) ? 1 : num-new_num1-new_num2;
  }
  // recursive quicksort calls
  // pragmas are ONLY considered when invoked from parallel quicksort
  // if arraysize over 10000, start new tasks
#pragma omp task if ( distance1 > 10000 )
  quicksort_dual_pivot(first, middle11, cmp, new_num1);
#pragma omp task if ( distance2 > 10000 )
  quicksort_dual_pivot(middle12, middle21, cmp, new_num2);
#pragma omp task if ( distance3 > 10000 )
  quicksort_dual_pivot(middle22, last, cmp, new_num3);
#pragma omp taskwait
}

// parallel dual pivot quicksort starter
template< class FwdIt, class Compare = std::less<> >
void pquicksort_dual_pivot( const FwdIt first, const FwdIt last,
                            const Compare cmp = Compare{} )
{
#pragma omp parallel
#pragma omp single nowait
  quicksort_dual_pivot( first, last, cmp, omp_get_max_threads() );
}

// parallel quickselect
template< class FwdIt, class Compare = std::less<> >
void pquickselect( const FwdIt first, const FwdIt nth, const FwdIt last,
                   const Compare cmp = Compare{},
                   const int num = omp_get_max_threads() )
{
  if( first == last ) return;

  const int distance = std::distance( first, last );

  // median of three as pivot is more robust for natrual distributions
  const auto pivot = medianOfThree( *first, *std::prev( last, 1 ),
                                    *std::next( first, distance/2 ) );

  // two partitionings to avoid getting stucked
  const auto cmp1 = [=]( const auto &elem ){ return cmp( elem, pivot ); };
  const auto cmp2 = [=]( const auto &elem ){ return !cmp( pivot, elem ); };
  FwdIt middle1, middle2;

  // ppartitioning is more efficient for arrays not fitting in cache
  // spartitioning has less overhead once the array fitts in cache
  if( std::distance(first,last) >= 2*B )
  {
    middle1 = ppartition(first, last, cmp1, num);
    middle2 = ppartition(middle1, last, cmp2, num);
  }
  else
  {
    middle1 = spartition(first, last, cmp1);
    middle2 = spartition(middle1, last, cmp2);
  }

  // recursive quickselect calls
  if( nth < middle1 ) pquickselect( first, nth, middle1, cmp, num );
  else if( nth >= middle2 ) pquickselect( middle2, nth, last, cmp, num );
}
// parallel pquickselect as comparison
template< class FwdIt >
void pquickselect_iterativ( const FwdIt first, const FwdIt nth, const FwdIt last,
                            const int num = omp_get_max_threads() )
{
  FwdIt left = first;
  FwdIt right = last;
  while ( left < right)
  {
    const auto pivot = medianOfThree(*left, *(left+(right-left)/2), *(right-1));
    const auto p1 = [pivot](const auto& em){ return em < pivot; };
    const auto p2 = [pivot](const auto& em){ return em <= pivot; };

    FwdIt middle1;
    FwdIt middle2;

    if ( std::distance(right,left) >= 2*B ){
      middle1 = ppartition(left, right, p1);
      middle2 = ppartition(middle1, right, p2);
    }
    else {
      middle1 = spartition(left, right, p1);
      middle2 = spartition(middle1, right, p2);
    }

    if ( nth < middle1 ) right = middle1;
    else if ( nth >= middle2 ) left = middle2;
    else return;
  }
}

#endif // PPARTQUICK_HPP
