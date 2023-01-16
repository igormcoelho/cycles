#include <chrono>
#include <iostream>
//
#include <cycles/cycles_ptr.hpp>

// inspired from random bench for gcpp and tracked_ptr discussions

int main() {
  using namespace std::chrono;  // NOLINT
  using namespace cycles;       // NOLINT

  std::cout << "begin bench for cycles_ptr" << std::endl;
  auto c = high_resolution_clock::now();
  {
    // auto data = make_tracked<tracked_ptr<void>[]>(10000000);
    sptr<cycles_ctx> ctx{new cycles_ctx{}};
    std::vector<cycles_ptr<void>> data(10000000);
    for (int i = 0; i < 10000000; ++i) {
      // if (i % 1000) std::cout << "i=" << i << std::endl;
      switch (i % 6) {
        case 0:
          data[i] = cycles_ptr<char>::make(ctx);
          break;
        case 1:
          data[i] = cycles_ptr<int16_t>::make(ctx);
          break;
        case 2:
          data[i] = cycles_ptr<int>::make(ctx);
          break;
        case 3:
          data[i] = cycles_ptr<int64_t>::make(ctx);
          break;
        case 4:
          data[i] = cycles_ptr<float>::make(ctx);
          break;
        case 5:
          data[i] = cycles_ptr<double>::make(ctx);
          break;
      }
    }
  }
  std::cout
      << "cycles_ptr: "
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
