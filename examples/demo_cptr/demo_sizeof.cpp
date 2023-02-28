#include <map>
//
#include <cycles/detail/utils.hpp>
#include <cycles/detail/v1/TreeV1.hpp>
#include <cycles/relation_ptr.hpp>
#include <demo_cptr/Graph.hpp>
#include <pre-experiments/List.hpp>
#include <pre-experiments/nodes_exp.hpp>

using std::string, std::vector, std::map;

int main() {
  std::cout << "sizeof(List1) = " << sizeof(List1) << std::endl;
  std::cout << "sizeof(Node1) = " << sizeof(Node1) << std::endl;
  //
  std::cout << "sizeof(List<double>) = " << sizeof(List<double>) << std::endl;
  std::cout << "sizeof(Node<double>) = " << sizeof(LNode<double>) << std::endl;
  //
  std::cout << "sizeof(Node2_Variant) = " << sizeof(Node2_Variant) << std::endl;
  std::cout << "sizeof(Node3) = " << sizeof(Node3) << std::endl;
  std::cout << "sizeof(Node4_Herb) = " << sizeof(Node4_Herb) << std::endl;

  std::cout << "FINISHED!" << std::endl;
  return 0;
}