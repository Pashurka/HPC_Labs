using System;
using System.Diagnostics;
using System.Threading.Tasks;

class Program
{
    const long N = 2_500_000_000L;
    const int RUNS = 3;
    const int NUM_THREADS = 2;

    static void Main()
    {
        Console.OutputEncoding = System.Text.Encoding.UTF8;

        Console.WriteLine("Лабораторна робота №5");
        Console.WriteLine("Організація багатопотокових обчислень засобами бібліотеки TPL");
        Console.WriteLine("Решето Ератосфена");
        Console.WriteLine($"Діапазон: 2..{N}");
        Console.WriteLine($"Кількість потоків: {NUM_THREADS}");
        Console.WriteLine(new string('-', 70));

        int sqrtN = (int)Math.Sqrt((double)N);

        // Маленьке решето до √N — спільне для всіх запусків (не входить у вимірюваний час)
        bool[] small = BuildSmallSieve(sqrtN);

        // Розбиваємо "віртуальний" масив [0..N] на два bool[],
        // бо int-індекс не вміщає 2.5 млрд
        int half = (int)(N / 2);          // 1_250_000_000
        int rest = (int)(N - half);       // 1_250_000_000

        double totalTime = 0;

        for (int run = 1; run <= RUNS; ++run)
        {
            // Виділяємо масиви заново на кожен запуск, щоб вимірювання було чесним
            bool[] sieveLow = new bool[half + 1];
            bool[] sieveHigh = new bool[rest + 1];

            var sw = Stopwatch.StartNew();

            // Ініціалізація
            Array.Fill(sieveLow, true);
            Array.Fill(sieveHigh, true);
            sieveLow[0] = false; // 0
            sieveLow[1] = false; // 1

            // Паралельне закреслення кратних для кожного простого p ≤ √N
            Parallel.For(2, sqrtN + 1,
                new ParallelOptions { MaxDegreeOfParallelism = NUM_THREADS },
                p =>
                {
                    if (!small[p]) return;
                    long p2 = (long)p * p;
                    for (long m = p2; m <= N; m += p)
                        Set(sieveLow, sieveHigh, half, m, false);
                });

            // Підрахунок простих і максимального
            long count = 0, maxPrime = 0;
            for (long i = 2; i <= N; ++i)
                if (Get(sieveLow, sieveHigh, half, i)) { ++count; maxPrime = i; }

            sw.Stop();
            double elapsed = sw.Elapsed.TotalSeconds;
            totalTime += elapsed;

            Console.WriteLine(
                $"Запуск {run}: знайдено простих = {count}, " +
                $"максимальне просте = {maxPrime}, " +
                $"час = {elapsed:F3} секунд");

            // Звільняємо пам'ять перед наступним запуском
            sieveLow = null;
            sieveHigh = null;
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }

        Console.WriteLine(new string('-', 70));
        Console.WriteLine($"Середній час виконання: {totalTime / RUNS:F3} секунд");
    }

    // Маленьке решето до √N — дає ознаку "чи просте p"
    static bool[] BuildSmallSieve(int limit)
    {
        var isPrime = new bool[limit + 1];
        Array.Fill(isPrime, true);
        isPrime[0] = isPrime[1] = false;
        for (int p = 2; (long)p * p <= limit; ++p)
            if (isPrime[p])
                for (int m = p * p; m <= limit; m += p)
                    isPrime[m] = false;
        return isPrime;
    }

    // Доступ до "віртуального" масиву [0..N] через два bool[]-масиви
    static bool Get(bool[] lo, bool[] hi, int half, long idx)
        => idx <= half ? lo[idx] : hi[idx - half - 1];

    static void Set(bool[] lo, bool[] hi, int half, long idx, bool val)
    {
        if (idx <= half) lo[idx] = val;
        else hi[idx - half - 1] = val;
    }
}