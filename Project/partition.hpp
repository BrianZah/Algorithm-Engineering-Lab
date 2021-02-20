#ifndef PARTITION_HPP
#define PARTITION_HPP

#include <omp.h>

#include <iostream>
#include <atomic>

#include <algorithm>
#include <iterator>
#include <vector>

#define B 2048 // 4096



template <typename Iter>
void printVector2(Iter first, Iter last)
{
  for (Iter it = first; it < last; ++it)
    std::cout << *it << (it < last-1 ? ", " : "\n");
}


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
constexpr ForwardIt ppartition( const ForwardIt first, const ForwardIt last, const UnaryPredicate p )
{
  const auto num = omp_get_max_threads();

  const int N = last - first;
  std::atomic<int> numRemainingBlocks((N-1) / B + 1);
  std::atomic<int> i(0);
  std::atomic<int> j(0);
  if (N%B == 0) j++;

  //std::vector<int> test(numRemainingBlocks, 0);
  std::vector<long> remainingBlocks(num);

  std::atomic<int> LN (0);
  std::atomic<int> RN (0);
  int q = 0;

#pragma omp parallel reduction(+ : q)
{
  const auto pid = omp_get_thread_num();

  auto left_first = last;
  auto left_last = last;
  auto right_first = last;
  auto right_last = last;

  int ii, jj;
  int leftcounter = 0;
  int rightcounter = 0;

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

  bool receivedLastBlock = (right_first == (last - N%B));
  if (receivedLastBlock) right_last = last;

  while ( (left_first != last) && (right_first != last) )
  {
    auto result = neutralize(left_first, left_last, right_first, right_last, p);
    if (result%2 == 0)
    {
      //test[(left_first-first)/4] = -1;
      if (0 < std::atomic_fetch_sub(&numRemainingBlocks, 1)) {
        ii = std::atomic_fetch_add(&i, 1);
        left_first = first + ii*B;
        left_last = first + ii*B + B;
      } else {
        left_first = last;
        left_last = last;
      }
      leftcounter++;
    }
    if (result > 0)
    {
      //test[(right_first-first)/4] = 1;
      if (0 < std::atomic_fetch_sub(&numRemainingBlocks, 1)) {
        jj = std::atomic_fetch_add(&j, 1);
        right_first = last - N%B - jj*B;
        right_last = last - N%B - jj*B + B;
      } else {
        right_first = last;
        right_last = last;
      }
      rightcounter++;
    }
  }

  remainingBlocks[pid] = last - first;
  if (left_first != last)
  {
    remainingBlocks[pid] = left_first - first;
    q++;
  } else if (right_first != last) {
    remainingBlocks[pid] = right_first - first;
    q++;
  }

  if ( receivedLastBlock && right_first != last-N%B )
  {
    rightcounter--;
    auto tmp = N%B;
    RN += tmp;
  }

  leftcounter = leftcounter * B;
  rightcounter = rightcounter * B;

  LN += leftcounter;
  RN += rightcounter;
} // end parallel

  auto left_first = first;
  auto left_last = first + B;
  auto right_first = last - N%B;
  auto right_last = last;


  std::sort( remainingBlocks.begin(), remainingBlocks.end() );
/*
  std::sort( std::begin(remainingBlocks), std::end(remainingBlocks) );
  std::cout << "remainingBlocks: ";
  printVector2( std::begin(remainingBlocks), std::end(remainingBlocks) );
*/
/*
  std::cout << "\n 01 \n";
  std::cout << LN << " LN - RN " << RN << "\n";
  std::cout << "   remainingBlocks: ";
  printVector2( remainingBlocks.begin(), remainingBlocks.end() );
  std::cout << "   test: ";
  printVector2( test.begin(), test.end());
  for (auto it = first; it < last; it += B) printVector2(it, it+B);
*/
  long left = 0;
  long right = q-1;

  left_first = first + remainingBlocks[left];
  left_last = left_first + B;
  right_first = first + remainingBlocks[right];
  right_last = right_first + B;

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
/*
     else if (l < right || ended_balanced) { // l ist linker Block
        //std::cout << "left\n";
        right_first = first + LN;
        right_last = right_first + B;
        left_first = first + remainingBlocks[l];
        left_last = left_first + B;

        if ( right_first != left_first ) {
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
        LN += B;
      }
*/
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

template <class ForwardIt>
 void pquicksort(ForwardIt first, ForwardIt last)
 {
   if(std::distance(first,last) <= 32){
     insertion_sort(first, last);
     return;
   }
    auto pivot = *std::next(first, std::distance(first,last)/2);
    ForwardIt middle1 = spartition(first, last,
                         [pivot](const auto& em){ return em < pivot; });
    ForwardIt middle2 = spartition(middle1, last,
                         [pivot](const auto& em){ return em <= pivot; });
    pquicksort(first, middle1);
    pquicksort(middle2, last);
 }

 template <class ForwardIt>
  void quicksort(ForwardIt first, ForwardIt last)
  {
    if(std::distance(first,last) <= 32){
      insertion_sort(first, last);
      return;
    }
     auto pivot = *std::next(first, std::distance(first,last)/2);
     ForwardIt middle1 = std::partition(first, last,
                          [pivot](const auto& em){ return em < pivot; });
     ForwardIt middle2 = std::partition(middle1, last,
                          [pivot](const auto& em){ return em <= pivot; });
     pquicksort(first, middle1);
     pquicksort(middle2, last);
  }


#endif // PARTITION_HPP
