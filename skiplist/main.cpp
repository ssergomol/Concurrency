#include <iostream>
#include <thread>
#include <vector>
#include "lock_based_skiplist.cpp"
#include "lock_free_skiplist.cpp"

std::string FILE_NAME = "lock_free_balanced";
int THREADS_NUMBER = 200;
int ITER_NUMB = 200;
int READS_NUMB = 180;
int INSERTION_NUMB = 10;
int DELETIONS_NUMB = 10;

LockFreeSkipList skiplist = LockFreeSkipList();

void pusher(int threadNumber) {
    std::cout << "add\n";
    for (int i = 3 * THREADS_NUMBER + threadNumber * INSERTION_NUMB;
    i < 3 * THREADS_NUMBER + (threadNumber + 1) * INSERTION_NUMB; i++) {
        skiplist.add(i);
    }
}

void popper(int threadNumber) {
    std::cout << "remove\n";
    for (int i = threadNumber * DELETIONS_NUMB; i < (threadNumber + 1) * DELETIONS_NUMB; i++) {
        skiplist.remove(i);
    }
}

void checker(int threadNumber) {
    std::cout << "check\n";
    for (int i = threadNumber * DELETIONS_NUMB; i < (threadNumber + 1) * DELETIONS_NUMB; i++) {
        skiplist.contains(i);
    }
}

int main(int argc, char **argv) {
    if (argc == 2) {
        THREADS_NUMBER = std::atoi(argv[argc - 1]);
    }

    for (int i = 0; i < 3 * THREADS_NUMBER * ITER_NUMB; i += 2) {
        skiplist.add(i);
    }



    std::vector<std::thread> threads1(THREADS_NUMBER);
    std::vector<std::thread> threads2(THREADS_NUMBER);
    std::vector<std::thread> threads3(THREADS_NUMBER);
//
    const auto startTime = std::chrono::high_resolution_clock::now();
//    for (int i = 0; i < 3 * THREADS_NUMBER; i += 3) {
//        threads[i] = std::thread(pusher, 3 * THREADS_NUMBER * ITER_NUMB + i, INSERTIONS * float(ITER_NUMB));
//        threads[i+1] = std::thread(popper, i, DELETIONS * float(ITER_NUMB));
//        threads[i+2] = std::thread(checker, 3 * THREADS_NUMBER - i, READS * float(ITER_NUMB));
//    }
    for (int i = 0; i < THREADS_NUMBER; i++) {
        threads1[i] = std::thread(pusher, i);
        threads2[i] = std::thread(popper, i);
        threads3[i] = std::thread(checker, i);
    }



//    for (int i = 0; i < THREADS_NUMBER; i++) {
//        threads[i] = std::thread(pusher, i);
//    }
//
//    for (int i = 0; i < THREADS_NUMBER; i++) {
//        if (threads[i].joinable()) {
//            threads[i].join();
//        }
//    }
//
//    for (int i = THREADS_NUMBER; i < 2 * THREADS_NUMBER; i++) {
//        threads[i] = std::thread(popper, i - THREADS_NUMBER);
//    }
//
//    for (int i = THREADS_NUMBER; i < 2 * THREADS_NUMBER; i++) {
//        if (threads[i].joinable()) {
//            threads[i].join();
//        }
//    }

    for (auto &thread : threads1) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    for (auto &thread : threads2) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    for (auto &thread : threads3) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    const auto endTime = std::chrono::high_resolution_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime-startTime);

    printf("Total time: %lld us\n", totalTime.count());

//    FILE *file = fopen(FILE_NAME.c_str(), "ab+");
//    long long buffer[2] { (long long) THREADS_NUMBER, totalTime.count()};
//    auto nBytes = fwrite(buffer, sizeof(buffer[0]), 2, file);
//    printf("\n\nWritten %lu bytes to %s\nthreads number: %lld time: %lld\n\n", nBytes, FILE_NAME.c_str(), buffer[0], buffer[1]);
//    fclose(file);

//    FILE *file2 = fopen("eff_ticketlock_avg", "ab+");
//    buffer[0] = (double) THREADS_NUMBER;
//    buffer[1] = total / THREADS_NUMBER;
//    nBytes = fwrite(buffer, sizeof(buffer[0]), 2, file2);
//    printf("\n\nWritten %lu bytes to %s\nthreads number: %lf time: %lf\n\n", nBytes, "eff_spinlock_output_average", buffer[0], buffer[1]);
//    fclose(file2);
    return 0;
}
