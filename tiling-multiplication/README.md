# Tiling matrix multiplication

There is cache-efficient implementation of matrix multiplication also known as tiling matrix multiplication. The main idea is to split the matrix into blocks and independently calculate result for each of them by executing threads with demanded task. Such matrix breaking allows processor to fit blocks entirely in cache which speeds up the calculations. The second motivation to use such implementation is the total number of elements' accesses which is much more less than it used to be in trivial case of multiplication
