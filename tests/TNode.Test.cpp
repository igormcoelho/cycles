
// #define CATCH_CONFIG_MAIN // This tells Catch to provide a main()
#include <iostream>
//
#include <catch2/catch_amalgamated.hpp>
#include <cycles/cycles_ptr.hpp>
#include <cycles/detail/TNode.hpp>

using namespace std;             // NOLINT
using namespace cycles;          // NOLINT
using namespace cycles::detail;  // NOLINT

// =======================
// memory management tests
// =======================

TEST_CASE("CyclesTestTNode: type erased node") {
  std::cout << "begin  type erased node" << std::endl;
  // NOLINTNEXTLINE
  auto data = TNodeData::make<double>(new double{1.0});
  std::stringstream ss;
  ss << data;
  std::string str = "TNodeData(1)";
  REQUIRE(str == ss.str());
}
