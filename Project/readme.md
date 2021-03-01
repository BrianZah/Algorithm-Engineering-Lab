# What's the project about?
The goal of the project was to implement a parallel version of a partitioner, quicksort and quickselect based on the findings from Philipas Tsigas and Yi Zhang.
# What is needed?
The library was tested with the GNU g++ and the Intel icpc compiler. To use the library you need a compiler with openmp support. If used within a CMake project, a current version of it is required as well. Older versions of CMake may have issues with including openMP in this way.
# How to use it
This library can be used as any other header-only library. The actual header is stored in the **lib** directory. Three common ways of using it are described in the following. 
## In a one folder project
The header can be downloaded and placed in the same folder as the main program is stored. Now, include the header with #include "ppartquick.hpp". After that, you should be able to use the functionality of this header. Do not forget to add the openMP directive when compiling and linking. A possible compile and link command 
may look as follows:<br/><br/>
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
Please, replace the corresponding file- and foldernames, eventually.
## Together with the test-examples
The complete project folder can also be downloaded to compare the performance with the gnu_parallel library.
For further details, have a look in the provided CMake script.
