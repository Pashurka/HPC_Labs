#include <iostream>
#include <vector>
#include <iomanip>
#include <windows.h>
#include <cmath>
#include <chrono>
#include <thread>

using namespace std;
// Функція для отримання кількості доступних процесорів
int get_num_processors() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}

// Функція для багатопотокового решета Ератосфена
void segmented_sieve(vector<char>& is_prime, long long start, long long end,
    long long sqrt_n, int thread_id) {
    // Спочатку знаходимо прості числа до sqrt(n) в основному потоці
    // Далі викреслюємо складені числа в сегменті

    for (long long p = 2; p <= sqrt_n; ++p) {
        if (!is_prime[p]) continue;

        // Знаходимо перше кратне p в сегменті
        long long first_multiple = max(p * p, ((start + p - 1) / p) * p);

        // Викреслюємо всі кратні p в сегменті
        for (long long multiple = first_multiple; multiple <= end; multiple += p) {
            is_prime[multiple] = 0;
        }
    }
}

int main(int argc, char** argv) {
    // --- Налаштування консолі для коректного виводу українською ---
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    // --- Прискорюємо роботу стандартного введення/виведення ---
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const long long N = 2'500'000'000LL;    // верхня межа
    const int RUNS = 3;             // повторів на кожну стратегію
    double total_time = 0.0;    // Накопичувач сумарного часу усіх запусків

    cout << "Лабораторна робота №4\n";
    cout << "Організація багатопотокових обчислень\n";
    cout << "Решето Ератосфена для 1.." << N << "\n";
    // Визначаємо кількість потоків
    int num_threads = 4;
    cout << "Кількість процесорів: " << num_threads << "\n";

    // --- Цикл тестових запусків ---
    for (int run = 1; run <= RUNS; ++run) {
        // 1. Створюємо масив для позначення простих чисел
        vector<char> is_prime(N + 1, 1); // 1 — просте, 0 — складене
        is_prime[0] = is_prime[1] = 0;   // 0 і 1 не є простими числами

        // 2. Верхня межа перевірки дільників
        long long sqrt_n = (long long)floor(sqrt((long double)N));

        // 3. Спочатку знаходимо прості числа до sqrt(n) в основному потоці
        auto t0 = chrono::high_resolution_clock::now();
        for (long long p = 2; p <= sqrt_n; ++p) {
            if (!is_prime[p]) continue;
            for (long long multiple = p * p; multiple <= sqrt_n; multiple += p) {
                is_prime[multiple] = 0;
            }
        }

        // 4. Розподіляємо роботу між потоками
        vector<thread> threads;
        long long segment_size = (N - sqrt_n + 1) / num_threads;

        for (int i = 0; i < num_threads; ++i) {
            long long start = sqrt_n + 1 + i * segment_size;
            long long end = (i == num_threads - 1) ? N : start + segment_size - 1;

            threads.emplace_back(segmented_sieve, ref(is_prime), start,
                end, sqrt_n, i);
        }

        // 5. Чекаємо на завершення всіх потоків
        for (auto& t : threads) {
            t.join();
        }

        // 6. Засікаємо час завершення обчислень
        auto t1 = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = t1 - t0;
        total_time += elapsed.count();

        // 7. Підрахунок кількості простих чисел та максимального простого
        long long count = 0;
        long long max_prime = 2;
        for (long long i = 2; i <= N; ++i) {
            if (is_prime[i]) {
                ++count;
                max_prime = i;
            }
        }

        // 8. Вивід результатів поточного запуску
        cout << "Запуск " << run
            << ": знайдено простих = " << count
            << ", максимальне просте = " << max_prime
            << ", час = " << fixed << setprecision(6)
            << elapsed.count() << " секунд\n";
    }

    // 9. Обчислення та вивід середнього часу виконання усіх запусків
    cout << "Середній час виконання: "
        << fixed << setprecision(6) << total_time / RUNS << " секунд\n";

    return 0;
}
