#include <iostream>     // Стандартне введення/виведення
#include <vector>       // Динамічні масиви 
#include <iomanip>      // Маніпулятори форматування виводу 
#include <windows.h>    // Налаштування кодування консолі Windows 
#include <cmath>        // Математичні функції 
#include <chrono>       // Вимірювання часу виконання 
#include <omp.h>        // Бібліотека OpenMP

using namespace std;

int main(int argc, char** argv) {
    // Налаштування консолі для коректного виводу українською
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    long long N = 2500000000;   // Верхня межа пошуку простих чисел
    const int RUNS = 3;          // Кількість повторних запусків
    double total_time = 0.0;     // Накопичувач сумарного часу усіх запусків

    // 0. Загальна інформація
    // Компіляція у командному рядку: 
    // g++ -O2 -fopenmp sieve_openmp.cpp -o sieve_openmp
    // Запуск виконувати без аргументів
    cout << "Лабораторна робота №2" << "\n";
    cout << "OpenMP-версія решета Ератосфена для діапазону 1-" << N << "\n";

    // Вручну задаємо кількість потоків OpenMP 
    int num_threads = 12;
    cerr << "Кількість потоків: " << num_threads << "\n";

    // --- Цикл тестових запусків ---
    for (int run = 1; run <= RUNS; ++run) {
        vector<char> is_prime(N + 1, 1); // 1 — просте, 0 — складене
        is_prime[0] = is_prime[1] = 0;

        long long limit = (long long)floor(sqrt((long double)N));
        auto t0 = chrono::high_resolution_clock::now();

        // Основний цикл решета Ератосфена
        for (long long p = 2; p <= limit; ++p) {
            if (!is_prime[p]) continue;
            long long start = p * p;
            // Паралельне викреслювання кратних чисел
            #pragma omp parallel for schedule(static)
            for (long long multiple = start; multiple <= N; multiple += p) {
                is_prime[multiple] = 0;
            }
        }

        auto t1 = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = t1 - t0;
        total_time += elapsed.count();

        // Підрахунок кількості простих чисел та максимального з них
        long long count = 0;
        long long max_prime = 2;
        for (long long i = 2; i <= N; ++i) {
            if (is_prime[i]) {
                ++count;
                max_prime = i;
            }
        }

        // Вивід результатів поточного запуску
        cout << "Запуск " << run
            << ": знайдено простих = " << count
            << ", максимальне просте = " << max_prime
            << ", час = " << fixed << setprecision(6)
            << elapsed.count() << " секунд\n";
    }

    // Середній час виконання усіх запусків
    cout << "Середній час виконання: "
        << fixed << setprecision(6) << total_time / RUNS << " секунд\n";

    return 0;
}
