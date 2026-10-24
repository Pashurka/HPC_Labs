#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <windows.h>
#include <iomanip>
#include <chrono>
#include <string>

#define CUDA_CHECK(call)                                                     \
    do {                                                                     \
        cudaError_t err_ = (call);                                           \
        if (err_ != cudaSuccess) {                                           \
            std::cerr << "CUDA error [" << __FILE__ << ":" << __LINE__       \
                      << "]: " << cudaGetErrorString(err_) << "\n";          \
            std::exit(EXIT_FAILURE);                                         \
        }                                                                    \
    } while (0)

// --- Simple CPU sieve for small primes up to limit ---
std::vector<int> simpleSieve(long long limit) {
    std::vector<char> isPrime(limit + 1, 1);
    isPrime[0] = isPrime[1] = 0;
    for (long long i = 2; i * i <= limit; ++i)
        if (isPrime[i])
            for (long long j = i * i; j <= limit; j += i)
                isPrime[j] = 0;

    std::vector<int> primes;
    for (long long i = 2; i <= limit; ++i)
        if (isPrime[i]) primes.push_back((int)i);
    return primes;
}

// --- Kernel: one block is responsible for one prime p = primes[blockIdx.x] ---
// Each thread of the block crosses out its own subset of multiples of p,
// starting from p^2.
__global__ void sieveKernel(unsigned char* __restrict__ sieve,
    long long N,
    const int* __restrict__ primes,
    int numPrimes) {
    const int primeIdx = blockIdx.x;
    if (primeIdx >= numPrimes) return;

    const long long p = primes[primeIdx];
    const long long start = p * p;
    if (start > N) return;

    for (long long j = start + (long long)threadIdx.x * p;
        j <= N;
        j += (long long)blockDim.x * p) {
        sieve[j] = 0;
    }
}

int main() {
    const long long N = 2500000000LL;

    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    std::ios::sync_with_stdio(false);

    std::cout << "Laboratory work No. 7. Parallel Sieve of Eratosthenes (CUDA)\n";
    std::cout << "Range: 1 - " << N << "\n";

    // --- exact integer sqrt(N) ---
    long long sqrtN = (long long)std::sqrt((double)N);
    while ((sqrtN + 1) * (sqrtN + 1) <= N) ++sqrtN;
    while (sqrtN * sqrtN > N) --sqrtN;

    // --- Step 1: small primes on CPU ---
    std::vector<int> primes = simpleSieve(sqrtN);

    // --- Step 2: reference buffer on host (1 = candidate, 0 = crossed out) ---
    const size_t totalBytes = (size_t)(N + 1);
    std::vector<unsigned char> h_sieve(totalBytes, 1);
    h_sieve[0] = 0;
    h_sieve[1] = 0;

    // --- Step 3: device buffers ---
    unsigned char* d_sieve = nullptr;
    CUDA_CHECK(cudaMalloc(&d_sieve, totalBytes));

    int* d_primes = nullptr;
    CUDA_CHECK(cudaMalloc(&d_primes, primes.size() * sizeof(int)));
    CUDA_CHECK(cudaMemcpy(d_primes, primes.data(),
        primes.size() * sizeof(int),
        cudaMemcpyHostToDevice));

    // --- Step 4: grid configuration - one block per prime ---
    dim3 grid((unsigned)primes.size());
    dim3 block(256);
    std::cout << "Blocks in grid: " << primes.size()
        << ", threads per block: " << block.x << "\n";

    float times[3] = { 0, 0, 0 };
    long long counts[3] = { 0, 0, 0 };
    long long maxPrimes[3] = { 0, 0, 0 };

    for (int test = 0; test < 3; ++test) {
        std::cout << "\n=== Test #" << (test + 1) << " ===\n";

        // Reset the device sieve to its initial state
        CUDA_CHECK(cudaMemcpy(d_sieve, h_sieve.data(), totalBytes,
            cudaMemcpyHostToDevice));

        cudaEvent_t evStart, evStop;
        CUDA_CHECK(cudaEventCreate(&evStart));
        CUDA_CHECK(cudaEventCreate(&evStop));

        CUDA_CHECK(cudaEventRecord(evStart));
        sieveKernel << <grid, block >> > (d_sieve, N, d_primes,
            (int)primes.size());
        CUDA_CHECK(cudaGetLastError());
        CUDA_CHECK(cudaEventRecord(evStop));
        CUDA_CHECK(cudaEventSynchronize(evStop));

        float ms = 0.0f;
        CUDA_CHECK(cudaEventElapsedTime(&ms, evStart, evStop));
        times[test] = ms / 1000.0f;
        std::cout << "Kernel execution time: " << times[test] << " s\n";

        CUDA_CHECK(cudaEventDestroy(evStart));
        CUDA_CHECK(cudaEventDestroy(evStop));

        // Copy result back to host
        CUDA_CHECK(cudaMemcpy(h_sieve.data(), d_sieve, totalBytes,
            cudaMemcpyDeviceToHost));

        // Count primes found in this test run (outside the timer)
        long long count = 0;
        long long maxPrime = 2;
        for (long long i = 2; i <= N; ++i) {
            if (h_sieve[i]) { ++count; maxPrime = i; }
        }
        counts[test] = count;
        maxPrimes[test] = maxPrime;

        std::cout << "Primes found: " << count << "\n";
        std::cout << "Largest prime: " << maxPrime << "\n";
    }

    std::cout << "\nSUMMARY\n";
    std::cout << std::fixed << std::setprecision(6);
    for (int test = 0; test < 3; ++test) {
        std::cout << "Test #" << (test + 1)
            << ": time = " << times[test] << " s"
            << ", primes = " << counts[test]
            << ", largest = " << maxPrimes[test] << "\n";
    }
    std::cout << "Average kernel time: "
        << (times[0] + times[1] + times[2]) / 3.0f << " s\n";

    CUDA_CHECK(cudaFree(d_sieve));
    CUDA_CHECK(cudaFree(d_primes));
    return 0;
}