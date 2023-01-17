
// #define CATCH_CONFIG_MAIN // This tells Catch to provide a main()
#include <iostream>
//
#include <catch2/catch_amalgamated.hpp>
#include <cycles/detail/TNode.hpp>
#include <cycles/relation_ptr.hpp>

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
  // print node
  std::stringstream ss;
  ss << data;
  std::string str = "TNodeData(1)";
  REQUIRE(str == ss.str());
  //
  {
    // begin scope for testing
    TNode<TNodeData> tnode{TNodeData::make_sptr<double>(new double{1.0})};
    tnode.debug_flag = true;
    REQUIRE(tnode_count == 1);
  }
  REQUIRE(tnode_count == 0);
}

TEST_CASE("CyclesTestTNode: type erased null node") {
  std::cout << "begin  type erased null node" << std::endl;
  // NOLINTNEXTLINE
  auto data = TNodeData::make<double>(nullptr);
  // print node
  std::stringstream ss;
  ss << data;
  std::string str = "TNodeData(NULL)";
  REQUIRE(str == ss.str());
  //
  {
    // begin scope for testing
    TNode<TNodeData> tnode{TNodeData::make_sptr<double>(nullptr)};
    tnode.debug_flag = true;
    REQUIRE(tnode_count == 1);
  }
  REQUIRE(tnode_count == 0);
}
