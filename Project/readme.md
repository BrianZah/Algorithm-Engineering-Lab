# What's the project about?
The goal of the project was to implement a parallel version of a partitioner, quicksort and quickselect based on the findings from Philipas Tsigas and Yi Zhang. (Philipas Tsigas and Yi Zhang. 2003. A Simple, Fast Parallel Implementation of Quicksort and its Performance. IEEE.)
# What is needed?
The library was tested with the GNU g++ and the Intel icpc compiler. To use the library a compiler with openmp support is needed. If used within a CMake project, a current version of it is required as well. Older versions of CMake may cause issues with including openMP in this way.
# How to install
This library can be used as any other header-only library. The actual header is stored in the **lib** directory. Three common ways of using it are described in the following. 
## In a one folder project
The header can be downloaded and placed in the same folder as the main program is stored. Now, the header can be included with #include "ppartquick.hpp". After that, you should be able to use the functionality of this header. Please, do not forget to add the openMP directive when compiling and linking. A possible compile and link command may look as follows:<br/><br/>
Intel compiler:
```bash
icpc main.cc -o main.exe -O2 -march=native -ffast-math -fopenmp
```
GNU compiler:
```bash
g++ main.cc -o main.exe -O2 -march=native -ffast-math -fopenmp
```
## In a CMake project
Create a lib directory in the projectfolder where is also stored the CMakeLists.txt. Now, insert the following lines into the CMakeLists.txt:<br/><br/>
```cmake
# add the path of the library to the CMake project
file( GLOB LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib" )
add_library( ppartquick INTERFACE )
target_include_directories( ppartquick INTERFACE "${LIB}" )
# openMP is required for the library
find_package( OpenMP REQUIRED )
# create the executable and add the libraries
add_executable( main.exe main.cc )
target_link_libraries( main.exe PRIVATE ppartquick )
target_link_libraries( main.exe PUBLIC OpenMP::OpenMP_CXX )
```
Eventually, replace the corresponding file- and foldernames.
## Together with the test-examples
The complete project folder can also be downloaded to compare the performance with the gnu_parallel library.
For further details, have a look at the provided CMake script.
# How to use
## adopting blocksize to system's L1-Cache
Please, adopt the blocksize to your system's L1-Cache. You will find further information in the first comment of the library (ppartquick.hpp).
## ppartition
```cpp
template< class FwdIt, class Predicate >
constexpr FwdIt ppartition( const FwdIt first, const FwdIt last,
                            const Predicate pred,
                            const int num = omp_get_max_threads(),
                            const bool omp_parallel_active = false );
```
- **ppartition** can be used as **std::partition** except for the option to give an execution policy. (https://en.cppreference.com/w/cpp/algorithm/partition)
- Additionally, the number of executing threads can be given.
- The parameter omp_parallel_active is for intern use.
## pqicksort and pquicksort_dual_pivot
```cpp
template< class FwdIt, class Compare = std::less<> >
void pquicksort( const FwdIt first, const FwdIt last, const Compare cmp = Compare{} );

template< class FwdIt, class Compare = std::less<> >
void pquicksort_dual_pivot( const FwdIt first, const FwdIt last, const Compare cmp = Compare{} );
```
- **pquicksort** and **pquicksort_dual_pivot** can be used as **std::sort** except for the option to give an execution policy. (https://en.cppreference.com/w/cpp/algorithm/sort)
- **pquicksort_dual_pivot** was in the experiments slower.
## pquickselect and pquickselect_iterativ
```cpp
template< class FwdIt, class Compare = std::less<> >
void pquickselect( const FwdIt first, const FwdIt nth, const FwdIt last,
                   const Compare cmp = Compare{},
                   const int num = omp_get_max_threads() );
                   
template< class FwdIt >
void pquickselect_iterativ( const FwdIt first, const FwdIt nth, const FwdIt last,
                            const int num = omp_get_max_threads() );
```
- **pquickselect** can be used as **std::nth_element** except for the option to give an execution policy. (https://en.cppreference.com/w/cpp/algorithm/nth_element)
- Additionally, the number of executing threads can be given.
- **pquickselect_iterativ** is significantly slower than pqickselect and does not offer to give a compare function as argument.
- The number of executing threads can be given.
# How tests were executed
First, special test cases were written. However, during the project, this approach turned out to be inefficient. Therefore, the test/test_with_gnu_parallel.cc was created. It allowed hundreds of thousands of randomly generated tests during the development process. Furthermore, this program also allows benchmarking with the gnu-parallel library.
