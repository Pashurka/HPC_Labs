#include <iostream>
#include <vector>
#include <iomanip>
#include <windows.h>
#include <chrono> 
#include <omp.h> 

using namespace std;

long long power_mod(long long base, long long exp, long long mod) {
    long long result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1) result = (result * base) % mod;
        base = (base * base) % mod;
        exp /= 2;
    }
    return result;
}

int main(int argc, char** argv) {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    long long N = 1030000000;
    const int RUNS = 3;
    double total_time = 0.0;

    cout << "Паралельна версія (OpenMP) аналізу чисел Калліна C_n = n * 2^n + 1\n";
    cout << "Кількість доступних потоків (ядер): " << omp_get_max_threads() << "\n";
    cout << "Діапазон індексів n: від 1 до " << N << "\n\n";

    for (int run = 1; run <= RUNS; ++run) {
        auto t0 = chrono::high_resolution_clock::now();

        long long count = 0;
        long long max_cullen_index = 0;

#pragma omp parallel 
        for (long long n = 1; n <= N; ++n) {
            long long mod3 = (n % 3 * power_mod(2, n, 3) + 1) % 3;

            if (mod3 == 0) {
                ++count;
                if (n > max_cullen_index) {
                    max_cullen_index = n;
                }
            }
        }

        auto t1 = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = t1 - t0;
        total_time += elapsed.count();

        cout << "Запуск " << run
            << ": знайдено чисел Калліна, кратних 3 = " << count
            << ", максимальний індекс n = " << max_cullen_index
            << ", час = " << fixed << setprecision(6)
            << elapsed.count() << " секунд\n";
    }

    cout << "\nСередній час виконання: "
        << fixed << setprecision(6) << total_time / RUNS << " секунд\n";

    return 0;
}
