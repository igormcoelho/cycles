
#include <map>
//
#include <demo_cptr/Graph.hpp>
#include <demo_cptr/XNode.hpp>
//
#include <cycles/detail/utils.hpp>
#include <cycles/detail/v1/TreeV1.hpp>
#include <cycles/relation_ptr.hpp>
#include <pre-experiments/List.hpp>
#include <pre-experiments/nodes_exp.hpp>

using std::string, std::vector, std::map;

using namespace cycles;          // NOLINT
using namespace cycles::detail;  // NOLINT

int main() {
  // tree
  //
  {
    std::cout << std::endl;
    std::cout << "========= " << std::endl;

    using NodeDouble =
        sptr<TNode<double>>;  /// typename TNode<double>::TNodeType;

    map<NodeDouble, sptr<Tree<double>>> mp;

    auto node1 = NodeDouble{new TNode<double>{sptr<double>{new double{2.0}}}};
    auto node2 = NodeDouble{new TNode<double>{sptr<double>{new double{3.0}}}};

    auto t1 = sptr<Tree<double>>(new Tree<double>{});
    t1->root = node1;
    std::cout << "t1.root = " << *t1->root << std::endl;
    // Tree<double> t2;
    auto t2 = sptr<Tree<double>>(new Tree<double>{});
    t2->root = node2;
    std::cout << "t2.root = " << *t2->root << std::endl;

    mp[node1] = t1;
    mp[node2] = t2;

    mp[node1]->print();
    mp[node2]->print();

    sptr<double> data1 = mp[node1]->get_root_data();
    sptr<double> data2 = mp[node2]->get_root_data();
    std::cout << "data1 = " << (data1 ? *data1 : -9999) << std::endl;
    std::cout << "data2 = " << (data2 ? *data2 : -9999) << std::endl;
    std::cout << "DROP Tree => ";
    t1->print();
    //
    t1 = nullptr;
    mp[node1] = nullptr;
    std::cout << "data1 = " << (data1 ? *data1 : -9999) << std::endl;
    std::cout << "data2 = " << (data2 ? *data2 : -9999) << std::endl;
  }
  //
  {
    std::cout << std::endl;
    std::cout << "========= " << std::endl;

    using NodeDouble = sptr<TNode<double>>;
    using TreeDouble = sptr<Tree<double>>;

    map<NodeDouble, TreeDouble> mp;

    auto node1 = NodeDouble(new TNode<double>{std::make_shared<double>(2.0)});
    auto node2 = NodeDouble(new TNode<double>{std::make_shared<double>(3.0)});

    auto t1 = TreeDouble(new Tree<double>{});
    t1->root = node1;
    std::cout << "t1.root = " << *t1->root->get_value() << std::endl;
    // Tree<double> t2;
    auto t2 = TreeDouble(new Tree<double>{});
    t2->root = node2;
    std::cout << "t2.root = " << *t2->root->get_value() << std::endl;

    mp[node1] = t1;
    mp[node2] = t2;

    mp[node1]->print();
    mp[node2]->print();
  }
  //

  std::cout << "FINISHED!" << std::endl;
  return 0;
}