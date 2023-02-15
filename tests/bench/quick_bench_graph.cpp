
// C++
#include <functional>
#include <set>
#include <string>
#include <vector>
//
#include <cycles/relation_ptr.hpp>
//
#include "../TestGraph.hpp"
//
// ================== EXAMPLE ================

using namespace cycles;  // NOLINT

int main() {
  std::cout << "rust_example1 (leaks due to cycle in sptr)" << std::endl;
  rust_example1::test_main();

  std::cout << "rust_example2 (no leaks due to arena)" << std::endl;
  rust_example2::test_main();

  std::cout << "cycles_example1 (no leaks due to relation_ptr)" << std::endl;
  cycles_example1::test_main();

  return 0;
}
