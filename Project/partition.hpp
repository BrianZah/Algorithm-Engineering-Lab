#ifndef PARTITION_HPP
#define PARTITION_HPP

#include <omp.h>

#include <iostream>
#include <atomic>

#include <iterator>
#include <vector>

#define B 2096


template< class ForwardIt, class UnaryPredicate >
constexpr int neutralize( ForwardIt left_first, ForwardIt left_last,
                 ForwardIt right_first, ForwardIt right_last, UnaryPredicate p )
{
  auto left_it = left_first;
  auto right_it = right_first;
  while ( left_it <= left_last && right_it <= right_last)
  {
    for ( ; left_it < left_last; left_it++ )
      if ( !p(*left_it) ) break;

    for ( ; right_it < right_last; right_it++ )
      if ( p(*right_it) ) break;

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

template <class ForwardIt>
void insertion_sort(ForwardIt first, ForwardIt last)
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

template< class ForwardIt, class UnaryPredicate >
constexpr ForwardIt spartition( const ForwardIt first, const ForwardIt last, const UnaryPredicate p )
{
  int split = 0;
  for (auto iter = first; iter < last; ++iter )
  {
    if ( p(*iter) ) ++split;
  }
  neutralize(first, first+split, first+split, last, p);
  return first+split;
}

template< class ForwardIt, class UnaryPredicate >
constexpr ForwardIt ppartition( const ForwardIt first, const ForwardIt last, const UnaryPredicate p, const int num = omp_get_max_threads() )
{
  //std::cout << "num = " << num << "\n";
  omp_set_num_threads(num);

  const int N = last - first;
  std::atomic<int> numRemainingBlocks((N-1) / B + 1);
  std::atomic<int> i(0);
  std::atomic<int> j(0);
  if (N%B == 0) j++;

  //std::vector<int> test(numRemainingBlocks, 0);
  std::vector<long> remainingBlocks(num);

  long LN = 0;
  long RN = 0;
  int q = 0;

#pragma omp parallel reduction(+ : LN, RN, q)
{
  const auto pid = omp_get_thread_num();

  auto left_first = last;
  auto left_last = last;
  auto right_first = last;
  auto right_last = last;

  int ii, jj;

  if (0 < std::atomic_fetch_sub(&numRemainingBlocks, 1)) {
    ii = std::atomic_fetch_add(&i, 1);
    left_first = first + ii*B;
    left_last = first + ii*B + B;
  } else {
    left_first = last;
    left_last = last;
  }

  if (0 < std::atomic_fetch_sub(&numRemainingBlocks, 1)) {
    jj = std::atomic_fetch_add(&j, 1);
    right_first = last - N%B - jj*B;
    right_last = last - N%B - jj*B + B;
  } else {
    right_first = last;
    right_last = last;
  }

  if (right_last > last) right_last = last;

  while ( (left_first != last) && (right_first != last) )
  {
    auto result = neutralize(left_first, left_last, right_first, right_last, p);
    if (result%2 == 0)
    {
      //test[(left_first-first)/4] = -1;
      LN += left_last-left_first;
      if (0 < std::atomic_fetch_sub(&numRemainingBlocks, 1)) {
        ii = std::atomic_fetch_add(&i, 1);
        left_first = first + ii*B;
        left_last = first + ii*B + B;
      } else {
        left_first = last;
        left_last = last;
      }
    }
    if (result > 0)
    {
      //test[(right_first-first)/4] = 1;
      RN += right_last -right_first;
      if (0 < std::atomic_fetch_sub(&numRemainingBlocks, 1)) {
        jj = std::atomic_fetch_add(&j, 1);
        right_first = last - N%B - jj*B;
        right_last = last - N%B - jj*B + B;
      } else {
        right_first = last;
        right_last = last;
      }
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

  insertion_sort( remainingBlocks.begin(), remainingBlocks.end() );
/*
  std::sort( std::begin(remainingBlocks), std::end(remainingBlocks) );
  std::cout << "remainingBlocks: ";
  printVector2( std::begin(remainingBlocks), std::end(remainingBlocks) );
*/

  long left = 0;
  long right = q-1;

  auto left_first = first + remainingBlocks[left];
  auto left_last = left_first + B;
  auto right_first = first + remainingBlocks[right];
  auto right_last = right_first + B;

  if (right_last > last) right_last = last;

  while ( left < right )
  {
    auto result = neutralize(left_first, left_last, right_first, right_last, p);
    if ( result%2 == 0 ) {
      //test[(left_first-first)/4] = -1;
      if ( left_first < first + LN + B ) {
        LN += B;
        remainingBlocks[left] = last - first;
      }
      left++;
      left_first = first + remainingBlocks[left];
      left_last = left_first + B;
    }
    if ( result > 0 ) {
      //test[(right_first-first)/4] = 1;
      if ( right_first >= last - RN - B ) {
        RN += right_last - right_first;
        remainingBlocks[right] = last - first;
      }
      right--;
      right_first = first + remainingBlocks[right];
      right_last = right_first + B;
    }
  }

  for ( int l = right; l < q; l++) {
    if (remainingBlocks[l] != last - first) {
      if (l > right) { // l ist rechter Block
        //std::cout << "right\n";
        left_first = last - RN - B; // erstes Element vom ersten linken Block von recht aus gesehen
        left_last = left_first + B;
        right_first = first + remainingBlocks[l];
        right_last = right_first + B;

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
        RN += B;
        for (int k = q-1; k > l; k--) {
          if (remainingBlocks[k] == N - RN - B) {
            remainingBlocks[k] = last - first;
            RN += B;
          }
        }
      }
    }
  }

  for ( int l = left; l >= 0 ; l--) {
    if (remainingBlocks[l] != last - first) {
      if (l < left) { // l ist rechter Block
        //std::cout << "right\n";
        right_first = first + LN;
        right_last = right_first + B;
        left_first = first + remainingBlocks[l];
        left_last = left_first + B;

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
        LN += B;
        for (int k = 0; k < l; k++) {
          if (remainingBlocks[k] == LN) {
            remainingBlocks[k] = last - first;
            LN += B;
          }
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

    if ( right_first != left_first ){
      if ( left_last > last) left_last = last;

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
  return spartition(first+LN, last-RN, p);
}

template< class Elem >
Elem medianOfThree(Elem a, Elem b, Elem c) {
    if ((a > b) xor (a > c))
        return a;
    else if ((b < a) xor (b < c))
        return b;
    else
        return c;
}

template <class ForwardIt>
void pquicksort_(ForwardIt first, ForwardIt last, const int num = omp_get_max_threads())
{
  if(std::distance(first,last) <= 32){
    insertion_sort(first, last);
    return;
  }
  const auto pivot = medianOfThree(*first, *(first+(last-first)/2), *(last-1));

  const auto p1 = [pivot](const auto& em){ return em < pivot; };
  const auto p2 = [pivot](const auto& em){ return em <= pivot; };

  ForwardIt middle1;
  ForwardIt middle2;

  if ( std::distance(first,last) >= 2*B ){
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
    const float share = 1.0 * (middle1-first) / ((middle1-first)+(last-middle2));
    new_num1 = ((int)(share * num) < 1) ? 1 : share * num;
    new_num2 = ((num-new_num1) < 1) ? 1 : num-new_num1;
  }

#pragma omp task
  pquicksort_(first, middle1, new_num1);
#pragma omp task
  pquicksort_(middle2, last, new_num2);
}

template <class ForwardIt>
void pquicksort(ForwardIt first, ForwardIt last, const int num = omp_get_max_threads())
{
  omp_set_nested(1);
  omp_set_dynamic(0);
#pragma omp parallel
#pragma omp single
  pquicksort_(first, last, num);
}

template< class ForwardIt >
void pquickselect( const ForwardIt first, const ForwardIt nth, const ForwardIt last, const int num = omp_get_max_threads() )
{
  if ( first == last ) return;

  const auto pivot = medianOfThree(*first, *(first+(last-first)/2), *(last-1));
  const auto p1 = [pivot](const auto& em){ return em < pivot; };
  const auto p2 = [pivot](const auto& em){ return em <= pivot; };

  ForwardIt middle1;
  ForwardIt middle2;

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

#endif // PARTITION_HPP
