#include <chrono>
#include <iostream>
//
#include <cycles/relation_ptr.hpp>

// inspired from random bench for gcpp and tracked_ptr discussions

int main() {
  using namespace std::chrono;  // NOLINT
  using namespace cycles;       // NOLINT

  std::cout << "begin bench for relation_ptr" << std::endl;
  auto c = high_resolution_clock::now();
  {
    // auto data = make_tracked<tracked_ptr<void>[]>(10000000);
    // sptr<DynowForestV1> ctx{new DynowForestV1{}};
    relation_pool<> pool;
    std::vector<relation_ptr<void>> data(10000000);
    for (int i = 0; i < 10000000; ++i) {
      // if (i % 1000) std::cout << "i=" << i << std::endl;
      switch (i % 6) {
        case 0:
          data[i] = pool.make<char>();
          break;
        case 1:
          data[i] = pool.make<int16_t>();
          break;
        case 2:
          data[i] = pool.make<int>();
          break;
        case 3:
          data[i] = pool.make<int64_t>();
          break;
        case 4:
          data[i] = pool.make<float>();
          break;
        case 5:
          data[i] = pool.make<double>();
          break;
      }
    }
  }
  std::cout
      << "relation_ptr: "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  std::cout << "begin bench for shared_ptr" << std::endl;
  c = high_resolution_clock::now();
  {
    // auto data = std::make_shared<std::shared_ptr<void>[]>(10000000);
    std::vector<std::shared_ptr<void>> data(10000000);
    for (int i = 0; i < 10000000; ++i) {
      switch (i % 6) {
        case 0:
          data[i] = std::make_shared<char>();
          break;
        case 1:
          data[i] = std::make_shared<int16_t>();
          break;
        case 2:
          data[i] = std::make_shared<int>();
          break;
        case 3:
          data[i] = std::make_shared<uint64_t>();
          break;
        case 4:
          data[i] = std::make_shared<float>();
          break;
        case 5:
          data[i] = std::make_shared<double>();
          break;
      }
    }
  }
  std::cout
      << "shared_ptr: "
      << duration<double, std::milli>(high_resolution_clock::now() - c).count()
      << "ms" << std::endl;

  return 0;
}
