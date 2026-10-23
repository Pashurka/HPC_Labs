#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <string>
#include <windows.h>
#include <omp.h>

using namespace std;
using ll = long long;

// Відома кількість простих чисел <= 500'000'000 (еталон для перевірки)
static const ll EXPECTED_PRIMES = 121443371LL;
// Один прогін решета Ератосфена з вказаною стратегією планування.
void run_sieve(const string& sch, ll N, int num_threads,
    ll& out_count, ll& out_max, double& out_time)
{
    // --- Розбір рядка: "name" або "name,chunk" ---
    string name = sch;
    int chunk = 0;                         // 0 => chunk не вказано
    size_t comma = sch.find(',');
    if (comma != string::npos) {
        name = sch.substr(0, comma);
        chunk = stoi(sch.substr(comma + 1));
    }

    vector<char> is_prime(N + 1, 1);       // 1 — просте, 0 — складене
    is_prime[0] = is_prime[1] = 0;

    ll limit = (ll)floor(sqrt((long double)N));

    auto t0 = chrono::steady_clock::now();

    for (ll p = 2; p <= limit; ++p) {
        if (!is_prime[p]) continue;
        ll start = p * p;

        if (name == "static") {
            if (chunk > 0) {
            #pragma omp parallel for schedule(static, chunk) num_threads(num_threads)
                for (ll m = start; m <= N; m += p) is_prime[m] = 0;
            }
            else {
            #pragma omp parallel for schedule(static) num_threads(num_threads)
                for (ll m = start; m <= N; m += p) is_prime[m] = 0;
            }
        }
        else if (name == "dynamic") {
            if (chunk > 0) {
            #pragma omp parallel for schedule(dynamic, chunk) num_threads(num_threads)
                for (ll m = start; m <= N; m += p) is_prime[m] = 0;
            }
            else {
            #pragma omp parallel for schedule(dynamic) num_threads(num_threads)
                for (ll m = start; m <= N; m += p) is_prime[m] = 0;
            }
        }
        else if (name == "guided") {
            if (chunk > 0) {
            #pragma omp parallel for schedule(guided, chunk) num_threads(num_threads)
                for (ll m = start; m <= N; m += p) is_prime[m] = 0;
            }
            else {
            #pragma omp parallel for schedule(guided) num_threads(num_threads)
                for (ll m = start; m <= N; m += p) is_prime[m] = 0;
            }
        }
        else {
            cerr << "!!! Невідома стратегія: '" << sch << "'\n";
            exit(1);
        }
    }

    auto t1 = chrono::steady_clock::now();
    out_time = chrono::duration<double>(t1 - t0).count();

    out_count = 0; out_max = 2;
    for (ll i = 2; i <= N; ++i) {
        if (is_prime[i]) { ++out_count; out_max = i; }
    }
}

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    ios::sync_with_stdio(false);

    const ll  N = 2'500'000'000LL;    // верхня межа
    const int RUNS = 3;             // повторів на кожну стратегію
    const int num_threads = 12;     // фіксована кількість потоків

    cout << "Лабораторна робота №3\n";
    cout << "Дослідження впливу стратегій планування (schedule) "
        "на продуктивність паралельних алгоритмів\n";
    cout << "OpenMP-решето Ератосфена для 1.." << N << "\n";
    cout << "Кількість потоків: " << num_threads << "\n";
    cout << "Кількість тестових запусків на стратегію: " << RUNS << "\n\n";

    // --- Список досліджуваних стратегій ---
    vector<string> strategies = {
        "static",
        "dynamic,1000",
        "dynamic,10000",
        "dynamic,100000",
        "guided,1000",
        "guided,10000",
        "guided,100000",
    };

    for (const auto& sch : strategies) {
        double total = 0.0;
        ll count = 0, max_prime = 0;

        cout << "=== schedule(" << sch << ") ===\n";
        for (int run = 1; run <= RUNS; ++run) {
            double t; ll c, mp;
            run_sieve(sch, N, num_threads, c, mp, t);

			// --- Перевірка кількості знайдениз простих чисел 
            if (c != EXPECTED_PRIMES) {
                cerr << "!!! Помилка у стратегії " << sch
                    << ": отримано " << c
                    << " простих замість очікуваних "
                    << EXPECTED_PRIMES << "\n";
                exit(1);
            }

            total += t;
            count = c; max_prime = mp;

            cout << "  Запуск " << run << ": час = "
                << fixed << setprecision(6) << t << " с\n";
        }

        cout << "  Простих = " << count
            << ", максимальне = " << max_prime
            << ", середній час = " << fixed << setprecision(6)
            << total / RUNS << " с\n\n";
    }

    cout << "Усі стратегії дали однаковий результат — алгоритм коректний.\n";
    return 0;
}