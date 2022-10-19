#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <fcntl.h>

int TILE_DIM = 512;
int MATRIX_SIZE = 1536;
int THREADS_NUMBER = 250;
int i_step = 0;


void standardMult(double *A, double *B, double *C) {
	int i = i_step++;
	for (int j = 0; j < MATRIX_SIZE; j++) {
		for (int k = 0; k < MATRIX_SIZE; k++) {
			C[i * MATRIX_SIZE + j] += A[i * MATRIX_SIZE + k]
					* B[k * MATRIX_SIZE + j];
		}
	}
}

void stupidMultiplication(double *A, double *B, double *C) {
	for (size_t i = 0; i < MATRIX_SIZE; i++) {
		for (size_t j = 0; j < MATRIX_SIZE; j++) {
			for (size_t k = 0; k < MATRIX_SIZE; k++) {
				C[i * MATRIX_SIZE + j] += A[i * MATRIX_SIZE + k]
						* B[k * MATRIX_SIZE + j];
			}
		}
	}
}

void standardMultiplication(double *A, double *B, double *C,
		std::vector<std::thread> &threads) {
	while (i_step < MATRIX_SIZE) {
		if (threads.size() < THREADS_NUMBER) {
			threads[threads.size()] = std::thread(standardMult, A, B, C);
		} else {
			standardMult(A, B, C);
		}
	}

	for (auto &thread : threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
}

void tileMult(double *A, double *B, double *C, size_t i, size_t j, size_t k) {
	for (size_t iTile = 0; iTile < TILE_DIM; ++iTile) {
		for (size_t kTile = 0; kTile < TILE_DIM; ++kTile) {
			for (size_t jTile = 0; jTile < TILE_DIM; ++jTile) {
				C[(i + iTile) * MATRIX_SIZE + j + jTile] += A[(i + iTile)
						* MATRIX_SIZE + k + kTile]
						* B[(k + kTile) * MATRIX_SIZE + j + jTile];
			}
		}
	}
}

void concurrentTilingMultiplication(double *A, double *B, double *C,
		std::vector<std::thread> &threads) {
	for (size_t i = 0; i < MATRIX_SIZE; i += TILE_DIM) {
		for (size_t j = 0; j < MATRIX_SIZE; j += TILE_DIM) {
			for (size_t k = 0; k < MATRIX_SIZE; k += TILE_DIM) {
				if (threads.size() >= THREADS_NUMBER) {
					tileMult(A, B, C, i, j, k);
				} else {
					threads[threads.size()] = std::thread(tileMult, A, B, C, i, j, k);
				}
			}
		}

	}

	for (auto &thread : threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}

}

int main(int argc, char **argv) {
	if (argc == 2) {
		THREADS_NUMBER = std::atoi(argv[argc - 1]);
	}

	double *matrixA = new double[MATRIX_SIZE * MATRIX_SIZE];
	double *matrixB = new double[MATRIX_SIZE * MATRIX_SIZE];
	double *matrixC = new double[MATRIX_SIZE * MATRIX_SIZE];

	std::vector<std::thread> threads(THREADS_NUMBER);

	for (size_t i = 0; i < MATRIX_SIZE; i++) {
		for (size_t j = 0; j < MATRIX_SIZE; j++) {
			matrixC[i * MATRIX_SIZE + j] = 0;
		}
	}

	for (size_t i = 0; i < MATRIX_SIZE; i++) {
		for (size_t j = 0; j < MATRIX_SIZE; j++) {
			matrixA[i * MATRIX_SIZE + j] = rand() % 10;
		}
	}

	for (size_t i = 0; i < MATRIX_SIZE; i++) {
		for (size_t j = 0; j < MATRIX_SIZE; j++) {
			matrixB[i * MATRIX_SIZE + j] = rand() % 10;
		}
	}

	auto start = std::chrono::high_resolution_clock::now();
	concurrentTilingMultiplication(matrixA, matrixB, matrixC, threads);
	auto stop = std::chrono::high_resolution_clock::now();

	auto threadDuration = duration_cast<std::chrono::microseconds>(
			stop - start);

	FILE *file = fopen("output", "ab+");
	long long buffer[2] { (long long) THREADS_NUMBER, threadDuration.count() };
	auto nBytes = fwrite(buffer, sizeof(buffer[0]), 2, file);
	fclose(file);
	return 0;
}
