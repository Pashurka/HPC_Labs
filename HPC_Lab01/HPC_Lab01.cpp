#include <iostream>     // Стандартне введення/виведення
#include <vector>       // Динамічні масиви 
#include <iomanip>      // Маніпулятори форматування виводу 
#include <windows.h>    // Налаштування кодування консолі Windows 
#include <cmath>        // Математичні функції 
#include <chrono>       // Вимірювання часу виконання 

using namespace std;

int main(int argc, char** argv) {
    // --- Налаштування консолі для коректного виводу українською ---
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    // --- Прискорюємо роботу стандартного введення/виведення ---
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    long long N = 2500000000;   // Верхня межа пошуку простих чисел
    const int RUNS = 3;      // Кількість повторних запусків
    double total_time = 0.0; // Накопичувач сумарного часу усіх запусків

    // 0. Загальна інформація
    cout << "Лабораторна робота №1. Послідовна версія решета Ератосфена для діапазону 1-" << N << "\n";
    cout << "Послідовна версія решета Ератосфена для діапазону 1-" << N << "\n";

    // --- Цикл тестових запусків ---
    for (int run = 1; run <= RUNS; ++run) {
        // 1. Створюємо масив для позначення простих чисел
        vector<char> is_prime(N + 1, 1); // 1 — просте, 0 — складене
        is_prime[0] = is_prime[1] = 0;   // 0 і 1 не є простими числами

        // 2. Верхня межа перевірки дільників
        long long limit = (long long)floor(sqrt((long double)N));

        // 3. Засікаємо час початку обчислень
        auto t0 = chrono::high_resolution_clock::now();

        // 4. Основний цикл решета Ератосфена
        for (long long p = 2; p <= limit; ++p) {
            if (!is_prime[p]) continue; // Пропускаємо складені числа

            // Викреслюємо всі кратні числа починаючи з p^2
            for (long long multiple = p * p; multiple <= N; multiple += p) {
                is_prime[multiple] = 0;
            }
        }

        // 5. Засікаємо час завершення обчислень
        auto t1 = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = t1 - t0;
        total_time += elapsed.count(); // Додаємо до сумарного часу

        // 6. Підрахунок кількості простих чисел та максимального простого
        long long count = 0;
        long long max_prime = 2;
        for (long long i = 2; i <= N; ++i) {
            if (is_prime[i]) {
                ++count;           // Лічильник простих чисел
                max_prime = i;     // Останнє знайдене просте число = максимальне
            }
        }

        // 7. Вивід результатів поточного запуску
        cout << "Запуск " << run
            << ": знайдено простих = " << count
            << ", максимальне просте = " << max_prime
            << ", час = " << fixed << setprecision(6)
            << elapsed.count() << " секунд\n";
    }

    // 8. Обчислення та вивід середнього часу виконання усіх запусків
    cout << "Середній час виконання: "
        << fixed << setprecision(6) << total_time / RUNS << " секунд\n";

    return 0; // Успішне завершення програми
}
