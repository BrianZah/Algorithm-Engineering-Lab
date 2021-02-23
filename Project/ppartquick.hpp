#ifndef PPARTQUICK_HPP
#define PPARTQUICK_HPP

#include <omp.h>
#include <atomic>


#define B 2096


template< class FwdIt, class Predicate >
inline constexpr int neutralize( FwdIt left_first, FwdIt left_last,
                 FwdIt right_first, FwdIt right_last, Predicate pred )
{
  auto left_it = left_first;
  auto right_it = right_first;
  while ( left_it <= left_last && right_it <= right_last)
  {
    for ( ; left_it < left_last; left_it++ )
      if ( !pred(*left_it) ) break;

    for ( ; right_it < right_last; right_it++ )
      if ( pred(*right_it) ) break;

    if ( (left_it == left_last) || (right_it == right_last) ) break;

    auto tmp = *left_it;
    *left_it = *right_it;
    *right_it = tmp;

    ++left_it; ++right_it;
  }

  if ( (left_it == left_last) && (right_it == right_last) ) return 2;
  if ( left_it == left_last ) return 0;
  return 1;
}

template< class FwdIt >
inline void getLeftBlock( const FwdIt first, const FwdIt last,
                 FwdIt &left_first, FwdIt &left_last, std::atomic<int> &numRemainingBlocks, std::atomic<int> &i )
{
  if (0 < std::atomic_fetch_sub(&numRemainingBlocks, 1)) {
    auto ii = std::atomic_fetch_add(&i, 1);
    left_first = first + ii*B;
    left_last = first + ii*B + B;
  } else {
    left_first = last;
    left_last = last;
  }
}

template< class FwdIt >
inline void getRightBlock( const FwdIt first, const FwdIt last,
                 FwdIt &right_first, FwdIt &right_last, std::atomic<int> &numRemainingBlocks, std::atomic<int> &j, const long N )
{
  if (0 < std::atomic_fetch_sub(&numRemainingBlocks, 1)) {
    auto jj = std::atomic_fetch_add(&j, 1);
    right_first = last - N%B - jj*B;
    right_last = last - N%B - jj*B + B;
  } else {
    right_first = last;
    right_last = last;
  }
}

template< class FwdIt, class Predicate >
inline void parallel_phase( const FwdIt first, const FwdIt last,
                            const Predicate pred, const int num,
                            long &LN, long &RN, int &q, long *remainingBlocks )
{
  omp_set_num_threads(num);

  const long N = last - first;
  std::atomic<int> numRemainingBlocks((N-1) / B + 1);
  std::atomic<int> i(0);
  std::atomic<int> j(0);
  if (N%B == 0) j++;

#pragma omp parallel reduction(+ : LN, RN, q)
{
  const auto pid = omp_get_thread_num();

  FwdIt left_first, left_last, right_first, right_last;

  getLeftBlock ( first, last, left_first, left_last, numRemainingBlocks, i );
  getRightBlock ( first, last, right_first, right_last, numRemainingBlocks, j, N );
  if (right_last > last) right_last = last;

  while ( (left_first != last) && (right_first != last) )
  {
    auto result = neutralize(left_first, left_last, right_first, right_last, pred);
    if (result%2 == 0)
    {
      LN += left_last-left_first;
      getLeftBlock ( first, last, left_first, left_last, numRemainingBlocks, i );
    }
    if (result > 0)
    {
      RN += right_last - right_first;
      getRightBlock ( first, last, right_first, right_last, numRemainingBlocks, j, N );
    }
  }
  remainingBlocks[pid] = last - first;
  if (left_first != last) {
    remainingBlocks[pid] = left_first - first;
    q++;
  }
  else if (right_first != last) {
    remainingBlocks[pid] = right_first - first;
    q++;
  }
} // end parallel
}

template <class FwdIt>
inline void insertion_sort(FwdIt first, FwdIt last)
{
  int i = 1;
  while ( i < (last - first))
  {
    const auto x = *(first + i);
    int j = i-1;
    while (j >= 0 && *(first + j) > x)
    {
      *(first + j + 1) = *(first + j);
      --j;
    }
    *(first + j + 1) = x;
    ++i;
  }
}

template< class FwdIt >
inline void swapBlocks( FwdIt left_first, FwdIt left_last,
                 FwdIt right_first, FwdIt right_last )
{
  if ( right_first != left_first ) {
    auto tmp = *left_first;
    auto left_it = left_first;
    auto right_it = right_first;
    while (left_it < left_last && right_it < right_last) {
      tmp = *left_it;
      *left_it = *right_it;
      *right_it = tmp;
      left_it++;
      right_it++;
    }
  }
}

template< class FwdIt, class Predicate >
inline void sequ_neutralization( const FwdIt first, const FwdIt last,
                                 const Predicate pred, const int num, const int q,
                                 long &LN, long &RN, int &left, int &right, long *remainingBlocks )
{
  insertion_sort( remainingBlocks, remainingBlocks+num );

  const long N = last - first;
  left = 0;
  right = q-1;

  auto left_first = first + remainingBlocks[left];
  auto left_last = left_first + B;
  auto right_first = first + remainingBlocks[right];
  auto right_last = right_first + B;
  if (right_last > last) right_last = last;

  while ( left < right )
  {
    auto result = neutralize(left_first, left_last, right_first, right_last, pred);
    if ( result%2 == 0 ) {
      if ( left_first < first + LN + B ) {
        LN += B;
        remainingBlocks[left] = last - first;
      }
      left++;
      left_first = first + remainingBlocks[left];
      left_last = left_first + B;
    }
    if ( result > 0 ) {
      if ( right_first >= last - RN - B ) {
        RN += right_last - right_first;
        remainingBlocks[right] = last - first;
      }
      right--;
      right_first = first + remainingBlocks[right];
      right_last = right_first + B;
    }
  }
}

template< class FwdIt >
inline void sequ_swapping( const FwdIt first, const FwdIt last,
                           const int q, const int left, const int right,
                           long &LN, long &RN, long *remainingBlocks )
{
  const long N = last - first;
  FwdIt left_first, left_last, right_first, right_last;

  for ( int l = right+1; l < q; l++) { // right blocks
    if (remainingBlocks[l] != last - first) {
      left_first = last - RN - B;
      left_last = left_first + B;
      right_first = first + remainingBlocks[l];
      right_last = right_first + B;

      swapBlocks( left_first, left_last, right_first, right_last );
      RN += B;

      for (int k = q-1; k > l; k--) {
        if (remainingBlocks[k] == N - RN - B) {
          remainingBlocks[k] = last - first;
          RN += B;
        }
      }
    }
  }

  for ( int l = left-1; l >= 0 ; l--) { // left blocks
    if (remainingBlocks[l] != last - first) {

      right_first = first + LN;
      right_last = right_first + B;
      left_first = first + remainingBlocks[l];
      left_last = left_first + B;

      swapBlocks( left_first, left_last, right_first, right_last );
      LN += B;

      for (int k = 0; k < l; k++) {
        if (remainingBlocks[k] == LN) {
          remainingBlocks[k] = last - first;
          LN += B;
        }
      }
    }
  }

  if ( left == right )
  {
    right_first = first + LN;
    right_last = right_first + B;
    left_first = first + remainingBlocks[right];
    left_last = left_first + B;

    if ( left_last > last) left_last = last;

    swapBlocks( left_first, left_last, right_first, right_last );
  }
}

template< class FwdIt, class Predicate >
constexpr FwdIt spartition( const FwdIt first, const FwdIt last, const Predicate pred )
{
  int split = 0;
  for (auto iter = first; iter < last; ++iter )
  {
    if ( pred(*iter) ) ++split;
  }
  neutralize(first, first+split, first+split, last, pred);
  return first+split;
}

template< class FwdIt, class Predicate >
constexpr FwdIt ppartition( const FwdIt first, const FwdIt last, const Predicate pred, const int num = omp_get_max_threads() )
{
  long LN = 0;
  long RN = 0;
  int q = 0;
  int left, right;
  long remainingBlocks[num];

  parallel_phase( first, last, pred, num, LN, RN, q, remainingBlocks );

  sequ_neutralization( first, last, pred, num, q, LN, RN, left, right, remainingBlocks );
  sequ_swapping( first, last, q, left, right, LN, RN, remainingBlocks );

  return spartition(first+LN, last-RN, pred);
}

template < class Elem >
Elem medianOfThree(Elem a, Elem b, Elem c) {
    if ((a > b) xor (a > c))
        return a;
    else if ((b < a) xor (b < c))
        return b;
    else
        return c;
}

template < class FwdIt, class Compare = std::less<> >
void pquicksort_( FwdIt first, FwdIt last, const int num = omp_get_max_threads(),
                  const Compare cmp = Compare{})
{
  const int distance = std::distance(first,last);

  if(distance <= 32){
    insertion_sort(first, last);
    return;
  }
  const auto pivot = medianOfThree( *first, *std::prev(last, 1),
                                    *std::next(first, distance/2));
  const auto p1 = [=](const auto &elem){ return cmp(elem, pivot); };
  const auto p2 = [=](const auto &elem){ return !cmp(pivot, elem); };

  FwdIt middle1, middle2;

  if ( distance >= 2*B ){
    middle1 = ppartition(first, last, p1, num);
    middle2 = ppartition(middle1, last, p2, num);
  }
  else {
    middle1 = spartition(first, last, p1);
    middle2 = spartition(middle1, last, p2);
  }

  int new_num1 = num;
  int new_num2 = num;
  if ( num > 1 ) {
    const float share = 1.0 * std::distance(first, middle1)
                        / ( std::distance(first, middle1)
                          + std::distance(middle2, last) );
    new_num1 = ( (int)(share * num) < 1 ) ? 1 : share * num;
    new_num2 = ( (num - new_num1) < 1 ) ? 1 : num - new_num1;
  }

#pragma omp task if ( distance > 25000 )
  pquicksort_(first, middle1, new_num1);
#pragma omp task if ( distance > 25000 )
  pquicksort_(middle2, last, new_num2);
}

template <class FwdIt>
void pquicksort(FwdIt first, FwdIt last, const int num = omp_get_max_threads())
{
  //omp_set_max_active_levels(1);
  omp_set_nested(1);
  omp_set_dynamic(0);
#pragma omp parallel
#pragma omp single
  pquicksort_(first, last, num);
}

template< class FwdIt >
void pquickselect( const FwdIt first, const FwdIt nth, const FwdIt last, const int num = omp_get_max_threads() )
{
  if ( first == last ) return;

  const auto pivot = medianOfThree(*first, *(first+(last-first)/2), *(last-1));
  const auto p1 = [pivot](const auto& em){ return em < pivot; };
  const auto p2 = [pivot](const auto& em){ return em <= pivot; };

  FwdIt middle1;
  FwdIt middle2;

  if ( std::distance(first,last) >= 2*B ){
    middle1 = ppartition(first, last, p1);
    middle2 = ppartition(middle1, last, p2);
  }
  else {
    middle1 = spartition(first, last, p1);
    middle2 = spartition(middle1, last, p2);
  }

  if ( nth < middle1 ) pquickselect( first, nth, middle1 );
  else if ( nth >= middle2 ) pquickselect( middle2, nth, last );
}

template< class FwdIt >
void pquickselect_iterativ( const FwdIt first, const FwdIt nth, const FwdIt last, const int num = omp_get_max_threads() )
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

template <class FwdIt>
void pquicksort_dual_pivot_(FwdIt first, FwdIt last, const int num = omp_get_max_threads())
{
  const int distance = std::distance(first,last);

  if(distance <= 32){
    insertion_sort(first, last);
    return;
  }
  auto pivot1 = medianOfThree(*first, *(first+(last-first)/2), *(last-1));
  auto pivot2 = medianOfThree(*(first+(last-first)/2+1), *(first+3*((last-first)/4)), *(last-2));

  if ( pivot1 > pivot2 ){
    auto tmp = pivot1;
    pivot1 = pivot2;
    pivot2 = tmp;
  }

  const auto p11 = [pivot1](const auto& em){ return em < pivot1; };
  const auto p12 = [pivot1](const auto& em){ return em <= pivot1; };
  const auto p21 = [pivot2](const auto& em){ return em < pivot2; };
  const auto p22 = [pivot2](const auto& em){ return em <= pivot2; };

  FwdIt middle11;
  FwdIt middle12;
  FwdIt middle21;
  FwdIt middle22;

  if ( distance >= 4*B ){
    middle11 = ppartition(first, last, p11, num);
    middle12 = ppartition(middle11, last, p12, num);
    middle21 = ppartition(middle12, last, p21, num);
    middle22 = ppartition(middle21, last, p22, num);
  }
  else {
    middle11 = spartition(first, last, p11);
    middle12 = spartition(middle11, last, p12);
    middle21 = spartition(middle12, last, p21);
    middle22 = spartition(middle21, last, p22);
  }

  int new_num1 = num;
  int new_num2 = num;
  int new_num3 = num;
  if ( num > 1 ) {
    const int elems = (middle11-first) + (middle21-middle12) + (last-middle22);
    const float share1 = 1.0 * (middle11-first) / elems;
    const float share2 = 1.0 * (middle21-middle12) / elems;
    new_num1 = ((int)(share1 * num) < 1) ? 1 : share1 * num;
    new_num2 = ((int)(share2 * num) < 1) ? 1 : share2 * num;
    new_num3 = ((num-new_num1-new_num2) < 1) ? 1 : num-new_num1-new_num2;
  }

#pragma omp task if ( distance > 25000 )
  pquicksort_dual_pivot_(first, middle11, new_num1);
#pragma omp task if ( distance > 25000 )
  pquicksort_dual_pivot_(middle12, middle21, new_num2);
#pragma omp task if ( distance > 25000 )
  pquicksort_dual_pivot_(middle22, last, new_num3);
}

template <class FwdIt>
void pquicksort_dual_pivot(FwdIt first, FwdIt last, const int num = omp_get_max_threads())
{
  //omp_set_max_active_levels(1);
  omp_set_nested(1);
  omp_set_dynamic(0);
#pragma omp parallel
#pragma omp single
  pquicksort_dual_pivot_(first, last, num);
}

#endif // PPARTQUICK_HPP
