using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;

class Program
{
    const long N = 2_500_000_000L;    // верхня межа
    const int RUNS = 3;
    const int SEGMENT_SIZE = 1 << 20; // 1M чисел на сегмент (~1 МБ)

    // Просте решето до √N — дає список малих простих
    static List<int> SimpleSieve(int limit)
    {
        var isPrime = new bool[limit + 1];
        Array.Fill(isPrime, true);
        isPrime[0] = isPrime[1] = false;
        for (int p = 2; (long)p * p <= limit; ++p)
            if (isPrime[p])
                for (int m = p * p; m <= limit; m += p)
                    isPrime[m] = false;

        var primes = new List<int>();
        for (int i = 2; i <= limit; ++i)
            if (isPrime[i]) primes.Add(i);
        return primes;
    }

    // Обробка одного сегменту [low..high]
    static (long count, long maxPrime) ProcessSegment(
        long low, long high, List<int> smallPrimes)
    {
        int size = (int)(high - low + 1);
        var isPrime = new bool[size];
        Array.Fill(isPrime, true);

        foreach (int p in smallPrimes)
        {
            long p2 = (long)p * p;
            if (p2 > high) break;

            long first = Math.Max(p2, ((low + p - 1) / p) * (long)p);
            for (long m = first; m <= high; m += p)
                isPrime[m - low] = false;
        }

        long count = 0, maxPrime = 0;
        for (int i = 0; i < size; ++i)
        {
            long value = low + i;
            if (value < 2) continue;
            if (isPrime[i]) { ++count; maxPrime = value; }
        }
        return (count, maxPrime);
    }

    static void Main()
    {
        Console.OutputEncoding = System.Text.Encoding.UTF8;

        Console.WriteLine("Лабораторна робота №4");
        Console.WriteLine("Організація багатопотокових обчислень");
        Console.WriteLine($"Решето Ератосфена для 1..{N}");

        int numThreads = Environment.ProcessorCount;
        Console.WriteLine($"Кількість процесорів: {numThreads}");

        int sqrtN = (int)Math.Sqrt((double)N);
        var smallPrimes = SimpleSieve(sqrtN);
        Console.WriteLine($"Малих простих до √N = {sqrtN}: {smallPrimes.Count}");

        long totalSegments = (N - 2 + SEGMENT_SIZE - 1) / SEGMENT_SIZE;

        // Пул потоків TPL налаштовуємо під кількість ядер
        ThreadPool.SetMinThreads(numThreads, numThreads);

        double totalTime = 0;

        for (int run = 1; run <= RUNS; ++run)
        {
            long totalCount = 0;
            long globalMaxPrime = 2;

            var sw = Stopwatch.StartNew();

            // ---- Parallel.For: головна частина ----
            // Паралельний цикл по сегментах.
            // Параметр parallelOptions.MaxDegreeOfParallelism задає к-сть потоків.
            // Параметр 'state' дозволяє зупинити цикл при потребі (не використовуємо).
            Parallel.For(
                fromInclusive: 0L,
                toExclusive: totalSegments,
                parallelOptions: new ParallelOptions
                {
                    MaxDegreeOfParallelism = numThreads
                },
                localInit: () => (count: 0L, maxPrime: 0L),
                body: (segIdx, state, local) =>
                {
                    long low = 2 + segIdx * SEGMENT_SIZE;
                    long high = Math.Min(N, low + SEGMENT_SIZE - 1);

                    var (cnt, mp) = ProcessSegment(low, high, smallPrimes);

                    local.count += cnt;
                    if (mp > local.maxPrime) local.maxPrime = mp;
                    return local;
                },
                localFinally: local =>
                {
                    // Атомарне додавання локального результату в глобальний
                    System.Threading.Interlocked.Add(ref totalCount, local.count);

                    // Атомарний max через CompareExchange
                    long cur;
                    do
                    {
                        cur = Volatile.Read(ref globalMaxPrime);
                        if (local.maxPrime <= cur) break;
                    }
                    while (Interlocked.CompareExchange(
                        ref globalMaxPrime, local.maxPrime, cur) != cur);
                });
            // ---- кінець Parallel.For ----

            sw.Stop();
            double elapsed = sw.Elapsed.TotalSeconds;
            totalTime += elapsed;

            Console.WriteLine(
                $"Запуск {run}: знайдено простих = {totalCount}, " +
                $"максимальне просте = {globalMaxPrime}, " +
                $"час = {elapsed:F6} секунд");
        }

        Console.WriteLine(
            $"Середній час виконання: {totalTime / RUNS:F6} секунд");
    }
}