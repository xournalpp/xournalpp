#include <benchmark/benchmark.h>

#include "MatrixV1.h"
#include "MatrixV2.h"

static void v1(benchmark::State& state) {
  using namespace xoj::util::v1;
  auto trans = xoj::util::v1::test_all;

  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    auto test = trans * trans;
    auto inv = test.inverse();
    auto res = inv * test;
    benchmark::DoNotOptimize(res);
  }
}
// Register the function as a benchmark
BENCHMARK(v1);

static void v2(benchmark::State& state) {
  using namespace xoj::util::v2;
  // Code before the loop is not measured
  auto trans = xoj::util::v2::test_all;

  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    auto test = trans * trans;
    auto inv = test.inverse();
    auto res = inv * test;
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(v2);

BENCHMARK_MAIN();