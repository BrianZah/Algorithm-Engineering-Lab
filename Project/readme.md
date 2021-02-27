# What's the project about?
The goal of the project was to implement a parallel version of a partitioner, quicksort and quickselect based on the findings from Phillipas Tsigas and Yi Zhang.
# How to use it
This library can be used as any other header-only library. The actual header is stored in the lib directory. Three common ways of how to use are described in the following.
## In a one folder project
The header can be downloaded and lay down in the same folder as the main program is stored. Include the header with #include "ppartquick.hpp". Now you should be able
to use the functionality of this header.
## In a CMake project
Create a lib directory in the projectfolder where is also stored the CMakeLists.txt. Now, insert the following lines into the CMakeLists.txt:<br/><br/>
file( GLOB LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib" )<br/>
add_library( ppartquick INTERFACE )<br/>
target_include_directories( ppartquick INTERFACE "${LIB}" )<br/>
find_package( OpenMP REQUIRED )<br/>
add_executable( main.exe main.cc )<br/>
target_link_libraries( main.exe PRIVATE ppartquick )<br/>
target_link_libraries( main.exe PUBLIC OpenMP::OpenMP_CXX )<br/><br/>
Replace the corresponding file- and foldernames, eventually.
## Together with the test-examples
The complete project folder can also be downloaded to compare the performance with the gnu_parallel library.
