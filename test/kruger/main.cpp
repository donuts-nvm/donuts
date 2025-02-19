#include <iostream>
#include <vector>
#include <ctime>
#include <chrono>
#include <random>

int main() {
   constexpr size_t SIZE = 1'000'000;
   constexpr size_t ITERATIONS = 2'500'000;
   constexpr size_t STEP = ITERATIONS / 10; // 10% de progresso

   std::cout << "Iniciando programa...\n";

   std::vector data(SIZE, 1);

   std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
   std::uniform_int_distribution<size_t> dist(0, SIZE - 1);

   long long sum = 0;

   std::cout << "Iniciando processamento...\n";

   const auto start = std::chrono::high_resolution_clock::now();

   for (size_t i = 0; i < ITERATIONS; ++i) {
      if (i % STEP == 0 && i != 0) {
         std::cout << (i * 100 / ITERATIONS) << "% concluído\n";
      }

      size_t index = dist(rng);
      sum += data[index];
      data[index] += 1;
   }

   const auto end = std::chrono::high_resolution_clock::now();

   std::chrono::duration<double> elapsed = end - start;

   std::cout << "100% concluído\n";
   std::cout << "Soma final: " << sum << "\n";
   std::cout << "Tempo decorrido: " << elapsed.count() << " segundos\n";

   return 0;
}