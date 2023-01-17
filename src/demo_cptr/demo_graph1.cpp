
#include <map>
//
#include <demo_cptr/Graph.hpp>
#include <demo_cptr/XNode.hpp>
//
#include <cycles/detail/Tree.hpp>
#include <cycles/detail/utils.hpp>
#include <cycles/relation_ptr.hpp>
#include <pre-experiments/List.hpp>
#include <pre-experiments/nodes_exp.hpp>

using std::string, std::vector, std::map;

int main() {
  // graph - part 1
  {
    std::cout << std::endl;
    std::cout << " -------- DEMO GRAPH1: SIMPLE GRAPH TESTING -------- "
              << std::endl;
    std::cout << std::endl;
    Graph g;
    g.vertex.push_back(Vertice{.label = "1"});
    g.vertex.push_back(Vertice{.label = "2"});
    //
    g.print();
    //

    // Is this a "CyclicArena"? An arena that has different lifetimes per
    // cycles, not the whole arena?
    vector<sptr<XNode>> all_nodes;
    //
    // register new node (through some factory "CyclicArena" method,
    // get_new_node()?)
    all_nodes.push_back(sptr<XNode>{new XNode{0}});
    all_nodes.push_back(sptr<XNode>{new XNode{1}});
    all_nodes.push_back(sptr<XNode>{new XNode{2}});
    all_nodes.push_back(sptr<XNode>{new XNode{3}});
    all_nodes.push_back(sptr<XNode>{new XNode{4}});
    all_nodes.push_back(sptr<XNode>{new XNode{5}});
    all_nodes.push_back(sptr<XNode>{new XNode{6}});
    //
    // DISPOSALS will only possible when this "main" node ref is dropped...
    // it means that real-time systems can take good care of these references,
    // and let the "internal world explode", when it's convenient
    //
    vector<List<XEdge<sptr<XNode>>>> cycles;
    List<XEdge<sptr<XNode>>> empty_list;
    cycles.push_back(empty_list);  // 0
    cycles.push_back(empty_list);  // 1

    vector<int> ref_cycles;
    ref_cycles.push_back(-1);  // 0
    ref_cycles.push_back(-1);  // 1
    ref_cycles.push_back(-1);  // 2

    //
    // create edge 1->2, where 1 is owner of 2
    //
    // does 1 have a cycle already? if not create one (HEAD to 1)
    cycles[1].push_back(all_nodes[1]);
    ref_cycles[1] = 1;  // OWNER is 1
    // if FROM 1 to destination 2 not have cycle, just add 2 after position of
    // node 1 in cycle 1
    cycles[1].push_back(all_nodes[2]);
    ref_cycles[2] = 1;  // OWNER is 1
    //
    // create edge 2->3, where 2 is owner of 3
    //
    // does 2 have a cycle already? Resp.: 1. So follow it, and put 3 from
    // position of node 2 in cycle 1 (just assuming last node is used here)
    cycles[1].push_back(all_nodes[3]);
    ref_cycles[3] = 1;  // OWNER is 1
    //
    //
    // create edge 3->6, where 3 is owner of 6
    //
    // does 3 have a cycle already? Resp.: 1. So follow it, and put 6 from
    // position of node 3 in cycle 1 (just assuming last node is used here)
    cycles[1].push_back(all_nodes[6]);

    //
    List<XEdge<XNode>> l_A;
    l_A.push_back(XNode{1});
    l_A.push_back(XNode{2});
    l_A.push_back(XNode{3});
    l_A.push_back(XEdge(XNode{6}, false));
    // l_A.push_front(XEdge(6, false));
    // l_A.push_front(3);
    // l_A.push_front(2);
    // l_A.push_front(1);
    std::cout << "X head to ";
    l_A.print();
    std::cout << "A: " << l_A.head.get() << std::endl;
    //
    List<XEdge<XNode>> l_B;
    l_B.push_front(XNode{6});
    l_B.push_front(XNode{13});
    l_B.push_front(XNode{12});
    l_B.push_front(XNode{8});
    l_B.push_front(XNode{7});
    l_B.push_front(XNode{10});
    std::cout << "head to ";
    l_B.print();
    std::cout << "B: " << l_B.head.get() << std::endl;
    //
    List<XEdge<XNode>> l_C;
    l_C.push_front(XEdge(XNode{8}, false));
    l_C.push_front(XNode{11});
    l_C.push_front(XNode{7});
    std::cout << "head to ";
    l_C.print();
    std::cout << "C: " << l_C.head.get() << std::endl;
    //
    List<XEdge<XNode>> l_D;
    l_D.push_front(XEdge(XNode{7}, false));
    l_D.push_front(XNode{3});
    std::cout << "head to ";
    l_D.print();
    std::cout << "D: " << l_D.head.get() << std::endl;
    //
    List<XEdge<XNode>> l_E;
    l_E.push_front(XEdge(XNode{3}, false));
    l_E.push_front(XNode{4});
    l_E.push_front(XNode{8});
    std::cout << "head to ";
    l_E.print();
    std::cout << "E: " << l_E.head.get() << std::endl;
    //
  }

  std::cout << "FINISHED!" << std::endl;
  return 0;
}