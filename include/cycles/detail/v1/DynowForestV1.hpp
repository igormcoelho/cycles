// SPDX-License-Identifier:  MIT
// Copyright (C) 2021-2022 - Cycles - https://github.com/igormcoelho/cycles

#ifndef CYCLES_DynowForestV1_HPP_  // NOLINT
#define CYCLES_DynowForestV1_HPP_  // NOLINT

// C++
#include <iostream>
#include <map>
#include <utility>
#include <vector>

//
#include <cycles/detail/IDynowForest.hpp>
//
#include <cycles/detail/utils.hpp>
#include <cycles/detail/v1/TArrowV1.hpp>
#include <cycles/detail/v1/TNodeV1.hpp>
#include <cycles/detail/v1/TreeV1.hpp>

using std::vector, std::ostream, std::map;  // NOLINT

// ========================
// relation_ptr and DynowForestV1
// ========================
// smart pointer suitable for cycles
// memory is self-managed
//-------------------------

namespace cycles {

namespace detail {

// NOLINTNEXTLINE
// class DynowForestV1 : public IDynowForest<TNode<TNodeData>, Tree<TNodeData>,
//                                           TArrowV1<TNodeData>> {
// NOLINTNEXTLINE
class DynowForestV1 : public IDynowForest<TArrowV1<TNodeData>> {
  // DynowForestV1 is type-erased by means of TNodeData
 public:
  // collect strategy parameters
  //
  bool _auto_collect{true};
  bool getAutoCollect() override { return _auto_collect; }
  bool setAutoCollect(bool ac) override {
    _auto_collect = ac;
    // if true, collect() now!
    if (ac) collect();
    // return 'true' if setAutoCollect(...) is supported
    return true;
  }
  //
  bool _debug{false};
  bool debug() override { return _debug; }
  void setDebug(bool d) override { _debug = d; }

 private:
  // Forest: every Tree is identified by its Root node in map system
  map<sptr<TNode<TNodeData>>, sptr<Tree<TNodeData>>> forest;

 public:
  // pending deletions of nodes
  vector<sptr<TNode<TNodeData>>> pending;

 private:
  bool is_destroying{false};

 public:
  DynowForestV1() {
    if (debug()) std::cout << "DynowForestV1 created!" << std::endl;
  }

  int getForestSize() override { return static_cast<int>(forest.size()); }

 public:
  // main operations

  sptr<TNodeData> op0_getSharedData(const TArrowV1<TNodeData>& arrow) override {
    sptr<TNode<TNodeData>> sremote_node = arrow.remote_node.lock();
    if (!sremote_node)
      return nullptr;
    else
      return sremote_node->value;
  }

  TArrowV1<TNodeData> op1_addNodeToNewTree(sptr<TNodeData> ref) override {
    // WE NEED TO HOLD SPTR locally, UNTIL we store it in definitive sptr tree
    sptr<TNode<TNodeData>> sptr_remote_node{new TNode<TNodeData>{ref}};
    //
    if (debug()) {
      std::cout << "=> C1 constructor: Registering this in new Tree!"
                << std::endl;
    }
    sptr<Tree<TNodeData>> stree(new Tree<TNodeData>{});
    if (debug()) {
      std::cout << "tree ~> ";
      stree->print();
      std::cout << "Printed Tree!" << std::endl;
    }

    // STRONG storage of remote node pointer
    stree->set_root(sptr_remote_node);
    this->forest[sptr_remote_node] = stree;
    // OK: 'sptr_remote_node' is free to go now
    if (debug()) this->print();
    //
    TArrowV1<TNodeData> arrow;
    arrow.owned_by_node = wptr<TNode<TNodeData>>{};
    arrow.remote_node = sptr_remote_node;
    return arrow;
  }

  // TArrowV1<TNodeData> op2_addChildStrong(sptr<TNode<TNodeData>> myNewParent,
  //                                        sptr<TNodeData> ref) override {
  TArrowV1<TNodeData> op2_addChildStrong(
      const TArrowV1<TNodeData>& arrowToParent, sptr<TNodeData> ref) override {
    auto myNewParent = arrowToParent.remote_node.lock();
    assert(myNewParent);  // TODO: remove // NOLINT
    // WE NEED TO HOLD SPTR locally, UNTIL we store it in definitive sptr tree
    sptr<TNode<TNodeData>> sptr_mynode{new TNode<TNodeData>{ref}};
    //
    // register STRONG ownership in tree
    //
    sptr_mynode->parent = myNewParent;
    myNewParent->add_child_strong(sptr_mynode);

    TArrowV1<TNodeData> arrow;
    arrow.owned_by_node = myNewParent;
    arrow.remote_node = sptr_mynode;
    arrow.is_owned_by_node = true;
    return arrow;
  }

  // TArrowV1<TNodeData> op3_weakSetOwnedBy(
  //     sptr<TNode<TNodeData>> this_remote_node,
  //     sptr<TNode<TNodeData>> owner_remote_node) override {
  TArrowV1<TNodeData> op3_weakSetOwnedBy(
      const TArrowV1<TNodeData>& arrowToOwned,
      const TArrowV1<TNodeData>& arrowToOwner) override {
    auto this_remote_node = arrowToOwned.remote_node.lock();
    auto owner_remote_node = arrowToOwner.remote_node.lock();
    assert(this_remote_node);   // TODO: remove // NOLINT
    assert(owner_remote_node);  // TODO: remove // NOLINT
    //
    // it seems that best logic is:
    // - each weak owned_by link corresponds to weak owns link
    // - each strong child link corresponds to a weak parent link
    //
    //
    if (debug()) {
      std::cout << std::endl
                << "relation_ptr:: unsafe_set_owned_by" << std::endl;
      std::cout << "TODO: Must register relation of:" << std::endl;
      std::cout << " this->remote_node=" << this_remote_node << ") '"
                << (this_remote_node->value.get())
                << "' owned_by:" << std::endl;
      std::cout << "\t"
                << " '" << (owner_remote_node->value.get()) << std::endl;
    }

    TNode<TNodeData>::add_weak_link_owned(this_remote_node, owner_remote_node);
    //
    if (debug())
      std::cout << "owner |children|=" << this_remote_node->children.size()
                << std::endl;
    //
    if (debug()) this->print();
    //
    TArrowV1<TNodeData> arrow;
    arrow.owned_by_node = owner_remote_node;
    arrow.remote_node = this_remote_node;
    arrow.is_owned_by_node = true;
    return arrow;
  }

  // NOLINTNEXTLINE
  void op4_remove(TArrowV1<TNodeData>& arc) override {
    bool isRoot = arc.is_root();
    bool isOwned = arc.is_owned();
    //
    assert(isRoot || isOwned);
    sptr<TNode<TNodeData>> owner_node = arc.owned_by_node.lock();
    sptr<TNode<TNodeData>> sptr_mynode = arc.remote_node.lock();
    // clear arc (???)
    arc.owned_by_node.reset();
    arc.remote_node.reset();

    //
    auto myctx = this;
    //
    bool will_die = myctx->op4x_checkSituationCleanup(sptr_mynode, owner_node,
                                                      isRoot, isOwned);
    //
    if (!will_die) {
      if (debug())
        std::cout << "DEBUG: WILL NOT DIE. CANNOT FORCE CLEAR HERE!"
                  << std::endl;
      return;
    }
    assert(will_die);
    // invoke expensive 'setNewOwner' operation
    will_die = myctx->op4x_setNewOwner(sptr_mynode);
    //
    // CLEAR!
    if (debug())
      std::cout << "CLEAR STEP: will_die = " << will_die << std::endl;

    myctx->op4x_prepareDestruction(sptr_mynode, owner_node, isRoot, isOwned);

    // final check: if will_die, send to pending list (FAST)
    if (will_die) myctx->op4x_destroyNode(sptr_mynode);
  }

 private:
  // OK - helper 1 of op4_remove
  bool op4x_checkSituationCleanup(sptr<TNode<TNodeData>> sptr_mynode,
                                  sptr<TNode<TNodeData>> owner_node,
                                  bool isRoot, bool isOwned) {
    bool will_die = false;
    //
    // AVOID Using TNode here...
    //    sptr_mynode->owned_by.size()
    if (debug()) {
      int owned_by_count = static_cast<int>(sptr_mynode->owned_by.size());
      std::cout << "destroy: |owns|=" << sptr_mynode->owns.size()
                << " |owned_by|=" << owned_by_count << std::endl;
    }
    //
    // check situation of node (if dying or not)
    //
    if (isRoot) {
      // this node is root in tree!
      if (debug()) std::cout << "DEBUG: I AM ROOT! I WILL DIE!" << std::endl;
      will_die = true;
    }  // is_root
    //
    if (isOwned) {
      // check if owner still exists
      if (debug()) std::cout << "DEBUG: I AM OWNED!" << std::endl;
      if (!owner_node) {
        // SHOULD THIS BEHAVE AS is_null?
        if (debug()) std::cout << "WARNING: avestruz!" << std::endl;
        will_die = false;
      } else {
        // CHECK IF OWNER IS MY PARENT...
        if (owner_node == sptr_mynode->parent.lock()) {
          if (debug())
            std::cout << "DEBUG: OWNER IS MY PARENT! I MAY DIE!" << std::endl;
          will_die = true;
        } else {
          if (debug())
            std::cout << "DEBUG: OWNER IS NOT MY PARENT! I WILL NOT DIE!"
                      << std::endl;
          // my node will stay alive since my parent still holds me strong
          will_die = false;
          // remove my weak link from owner
          bool r0 = TNodeHelper<>::removeFromOwnsList(owner_node, sptr_mynode);
          assert(r0);
          // remove owner from my weak link list
          bool r1 =
              TNodeHelper<>::removeFromOwnedByList(owner_node, sptr_mynode);
          assert(r1);
        }
      }
    }  // end is_owned
    return will_die;
  }

  // OK - helper 2 of op4_remove
  bool op4x_setNewOwner(sptr<TNode<TNodeData>> sptr_mynode) {
    bool will_die = true;  // default
    // auto myctx = this;
    if (debug())
      std::cout
          << "DEBUG: FIND OWNED_BY. NODE WILL DIE IF NOT FIND REPLACEMENT!"
          << std::endl;
    // find new owner, otherwise will die
    int owned_by_count = static_cast<int>(sptr_mynode->owned_by.size());
    //
    for (unsigned k = 0; k < owned_by_count; k++) {
      auto myNewParent = sptr_mynode->owned_by.at(k).lock();
      if (!myNewParent) {
        std::cout << "ERROR! no new parent! how??" << std::endl;
      }
      // new parent must exist
      assert(myNewParent);
      if (myNewParent.get() == sptr_mynode.get()) {
        if (debug()) {
          std::cout << "Found new parent to own me but it's loop! Ignoring! k="
                    << k << std::endl;
        }
        continue;
      }
      if (debug()) {
        std::cout
            << "Found new parent to own me (will check if not on subtree): "
            << myNewParent->value_to_string() << std::endl;
      }
      // NOTE: costly O(tree_size)=O(N) test in worst case for 'isDescendent'
      bool _isDescendent =
          TNodeHelper<>::isDescendent(myNewParent, sptr_mynode);
      //
      if (debug())
        std::cout << "DEBUG: isDescendent=" << _isDescendent << " k=" << k
                  << std::endl;
      if (_isDescendent) {
        if (debug())
          std::cout << "WARNING: owned_by is already my descendent! Discard. "
                    << "Will try next k!"
                    << "k=" << k << std::endl;
        // k++
        continue;
      }
      // found some good k!
      // assume this will not die (for this 'k'). FOUND some good owner!
      will_die = false;
      if (debug()) {
        std::cout << "Found new VALID parent to own me: "
                  << myNewParent->value_to_string() << std::endl;
      }
      // COSTLY. Remove me from the 'owns' list of my owner
      bool removed =
          TNodeHelper<>::removeFromOwnsList(myNewParent, sptr_mynode);
      assert(removed);
      // delete myself from owned_by (now I'm strong child)
      sptr_mynode->owned_by.erase(sptr_mynode->owned_by.begin() +
                                  k);  // TODO: terrible TNode here...
      // add myself as myNewParent child
      sptr_mynode->parent = myNewParent;
      myNewParent->add_child_strong(sptr_mynode);
      // will_die should be False, at this point
      if (!will_die) break;
    }  // end for k
    //
    return will_die;
  }

  // OK - helper 3 of op4_remove
  void op4x_prepareDestruction(sptr<TNode<TNodeData>> sptr_mynode,
                               sptr<TNode<TNodeData>> owner_node, bool isRoot,
                               bool isOwned) {
    auto myctx = this;
    // prepare final destruction
    if (debug())
      std::cout << "DEBUG: will check situation: either is_root or is_owned"
                << std::endl;
    if (isRoot) {
      if (debug())
        std::cout << "DEBUG: is_root. destroy_tree(...)" << std::endl;
      myctx->destroy_tree(sptr_mynode);
    }
    if (isOwned) {
      if (debug())
        std::cout << "DEBUG: is_owned. owner_node->remove_child(...)"
                  << std::endl;
      bool r = owner_node->remove_child(sptr_mynode.get());

      if (!r) std::cout << "SERIOUS WARNING: is this a LOOP node?" << std::endl;
      assert(r);
    }
  }

  // OK - helper 4 of op4_remove
  // NOLINTNEXTLINE
  void op4x_destroyNode(sptr<TNode<TNodeData>>& sptr_mynode) {
    auto myctx = this;
    if (debug())
      std::cout << "destroy: will_die is TRUE. MOVE TO GARBAGE." << std::endl;
    // MOVE NODE TO GARBAGE (DO NOT FIX CHILDREN NOW) - THIS MUST BE FAST
    assert(myctx->pending.size() == 0);
    // AVOID TNode here...
    // sptr_mynode->owned_by.size()
    //
    if (debug()) {
      int owned_by_count = static_cast<int>(sptr_mynode->owned_by.size());
      std::cout << "destroy: moving to pending list with these properties: ";
      std::cout << "node |owns|=" << sptr_mynode->owns.size()
                << " |owned_by|=" << owned_by_count << std::endl;
    }
    myctx->pending.push_back(std::move(sptr_mynode));
    int sz_pending = myctx->pending.size();
    if (debug()) std::cout << "DEBUG: sz_pending=" << sz_pending << std::endl;
    sptr_mynode = nullptr;  // NO EFFECT!
    //
    if (debug()) {
      std::cout << "destroy: in pending list with these properties: ";
      std::cout << "node |owns|=" << myctx->pending[sz_pending - 1]->owns.size()
                << " |owned_by|="
                << myctx->pending[sz_pending - 1]->owned_by.size() << std::endl;
    }
    // last holding reference to node is on pending list
    if (myctx->getAutoCollect()) {
      if (debug()) {
        std::cout << "DEBUG: begin auto_collect. |pending|="
                  << myctx->pending.size() << std::endl;
      }
      myctx->collect();
    }
  }

 public:
  // op5: receive 'arc' and make 'unowned' link
  TArrowV1<TNodeData> op5_copyNodeToNewTree(
      const TArrowV1<TNodeData>& arrow) override {
    // cannot get pointer from null or copy unowned
    if (arrow.is_null() || arrow.is_root()) {
      // return null
      TArrowV1<TNodeData> arr;
      assert(arr.is_null());
      return arr;
    }
    // only case supported by V1: copy of owned
    assert(arrow.is_owned());

    // (1) ensure that remote_node is NOT root of any existing tree...
    // Maybe this check could be avoided in the future, TODO: think
    // FIXED: in fact, we need to find if DATA is already there, right?
    // So, maybe we can adjust forest map in the future, to hold data->tree, not
    // node->tree...

    auto sptr_mynode = arrow.remote_node.lock();
    assert(sptr_mynode);

    // C++17 required (if C++11 is desired, change this part)
    bool found = false;
    for (auto const& [key, val] : this->forest) {
      if (key->value.get() == sptr_mynode->value.get()) {
        found = true;
        break;
      }
    }

    if (found) {
      // Cannot make double copy of unowned in this forest v1 structure.
      // Designed solution: return null
      TArrowV1<TNodeData> arr;
      assert(arr.is_null());
      return arr;
    }
    // GOOD: data in tree not existing, can make unowned copy

    // copy data sptr into new node
    sptr<TNode<TNodeData>> sptrNewNode{
        new TNode<TNodeData>{sptr_mynode->value}};

    // (2) create new Tree and make remote_node its root
    sptr<Tree<TNodeData>> stree(new Tree<TNodeData>{});
    // STRONG storage of remote node pointer
    stree->set_root(sptrNewNode);
    this->forest[sptrNewNode] = stree;

    // (3) must remove strong link from old parent to remote_node
    auto sptr_oldParent = arrow.owned_by_node.lock();
    assert(sptr_oldParent);

    bool r = sptr_oldParent->remove_child(sptr_mynode.get());
    if (!r)
      std::cout << "SERIOUS WARNING: is this a LOOP node (op5)?" << std::endl;
    assert(r);

    // (4) must include weak link from old parent to remote_node
    TNode<TNodeData>::add_weak_link_owned(sptr_mynode, sptr_oldParent);

    // (5) add strong child: sptrNewNode -> sptr_mynode (otherwise sptr_mynode
    // will die)

    //
    // register STRONG ownership in tree
    //
    sptr_mynode->parent = sptrNewNode;
    sptrNewNode->add_child_strong(sptr_mynode);

    // (6) create root arrow
    //
    TArrowV1<TNodeData> arr;
    arr.is_owned_by_node = false;
    arr.remote_node = sptrNewNode;
    assert(arr.is_root());
    // sanity action
    sptrNewNode = nullptr;
    // sanity action
    sptr_mynode = nullptr;
    //
    return arr;
  }

 public:
  void destroyAll() override {
    if (debug())
      std::cout << "DynowForestV1 destroy() forest_size =" << forest.size()
                << std::endl;
    destroyForestRoots();
    if (debug())
      std::cout << "~DynowForestV1: final cleanup on pending" << std::endl;
    assert(!is_destroying);
    // NOTE: collect is slower than destroy_pending with unchecked=true
    // collect();
    destroy_pending(true);
    if (debug())
      std::cout << "DynowForestV1 destroy(): finished final collect"
                << std::endl;
  }

  ~DynowForestV1() override {
    if (debug())
      std::cout << "~DynowForestV1() forest_size =" << forest.size()
                << std::endl;
    destroyAll();
    if (debug())
      std::cout << "~DynowForestV1: finished final collect" << std::endl;
  }

 private:
  // quickly destroy all forest roots
  void destroyForestRoots() {
    for (const auto& p : forest) {
      if (debug())
        std::cout << "destroyForestRoots: clearing root of ~> " << p.first
                  << "'" << (*p.first) << "' -> " << p.second << " TREE"
                  << std::endl;
      // NOLINTNEXTLINE
      assert(p.second->root);  // root must never be nullptr
      if (debug())
        std::cout << "destroyForestRoots: move tree root node to garbage for "
                     "deferred destruction"
                  << std::endl;
      // ===============================
      // DO NOT DESTROY RECURSIVELY HERE
      // p.second->root = nullptr;  // clear root BEFORE CHILDREN
      //
      // force clean both lists: owned_by and owns (UNCHECKED/FASTER)
      bool b1 = TNodeHelper<TNodeData>::cleanOwnsAndOwnedByLists(p.second->root,
                                                                 true);
      assert(b1);
      //
      // move to pending
      pending.push_back(std::move(p.second->root));
      p.second->root = nullptr;  // useless... just to make sure it's not here
    }
    if (debug())
      std::cout << "destroyForestRoots: final clear forest" << std::endl;
    forest.clear();  // is it necessary??
  }

 public:
  std::pair<int, int> debug_count_ownership_links() {
    std::pair<int, int> p{0, 0};
    for (auto& t : forest) {
      auto p1 = debug_count_owns_owned_by(t.second->root);
      p.first += p1.first;
      p.second += p1.second;
    }
    return p;
  }

 private:
  std::pair<int, int> debug_count_owns_owned_by(sptr<TNode<TNodeData>> node) {
    std::pair<int, int> p{0, 0};
    p.first += static_cast<int>(node->owns.size());
    p.second += static_cast<int>(node->owned_by.size());
    for (unsigned i = 0; i < node->children.size(); i++) {
      auto p1 = debug_count_owns_owned_by(node->children[i]);
      p.first += p1.first;
      p.second += p1.second;
    }
    return p;
  }

 private:
  void destroy_tree(sptr<TNode<TNodeData>> sptr_mynode) {
    if (debug()) std::cout << "destroy: will destroy my tree." << std::endl;
    // find my tree
    auto tree_it = this->forest.find(sptr_mynode);
    if (tree_it == this->forest.end()) {
      // ????
      std::cout << "ERROR! COULD NOT FIND MY TREE!" << std::endl;
      assert(false);
    } else {
      if (debug()) {
        std::cout << " ~~~> OK FOUND MY TREE. Delete it." << std::endl;
      }
      // clear tree
      this->forest.erase(tree_it);

      if (debug()) {
        std::cout << " ~~~> OK DELETED MY TREE." << std::endl;
      }
    }
  }

 public:
  // public method to manually invoke collection, if 'auto_collect' is not
  // true
  void collect() override { destroy_pending(false); }

 private:
  // destroy_pending(unchecked) performs destruction, with two modes:
  // - unchecked==false: cleans respecting/updating owns and owned_by lists
  // - unchecked==true:  cleans trees much faster, only destroying children
  // Internally, 'unchecked=false' should be always used, except on forest
  //   destructor, that allows reckless and faster destruction of everything.
  void destroy_pending(bool unchecked = false) {
    if (is_destroying) {
      if (debug())
        std::cout << "WARNING: collect() already executing!" << std::endl;
      return;
    } else {
      if (debug())
        std::cout << "CTX: NOT DESTROYING! BEGIN PROCESS!" << std::endl;
    }
    is_destroying = true;
    if (debug())
      std::cout << "CTX: destroy_pending. unchecked=" << unchecked << std::endl;
    if (debug())
      std::cout << "CTX: destroy_pending. |pending|=" << pending.size()
                << std::endl;
    if (pending.size() == 0) {
      is_destroying = false;
      return;
    }
    // ==============================
    //    begin destruction process
    // ==============================
    //
    // store data separately for delayed destruction
    std::vector<sptr<TNodeData>> vdata;

    // TODO(igormcoelho): make queue?
    while (pending.size() > 0) {
      if (debug()) {
        std::cout << std::endl;
        std::cout << "CTX: WHILE processing pending list. |pending|="
                  << pending.size() << std::endl;
      }
      sptr<TNode<TNodeData>> sptr_delete = std::move(pending[0]);
      pending.erase(pending.begin() + 0);
      //
      if (debug()) {
        std::cout << "CTX: sptr_delete is: " << sptr_delete->value_to_string()
                  << std::endl;
      }
      if (debug()) {
        std::cout << "CTX: sptr_delete with these properties: ";
        std::cout << "node |owns|=" << sptr_delete->owns.size()
                  << " |owned_by|=" << sptr_delete->owned_by.size()
                  << std::endl;
      }
      // this must be clean, regarding external (Except for cycles, maybe...)
      if (sptr_delete->owned_by.size() > 0) {
        // assert(sptr_delete->owned_by.size() == 0);
        if (debug())
          std::cout << "CTX WARNING: owned_by but dying... must be some cycle!"
                    << std::endl;
      }
      // force clean both lists: owned_by and owns (CHECKED OR UNCHECKED)
      bool b1 = TNodeHelper<TNodeData>::cleanOwnsAndOwnedByLists(sptr_delete,
                                                                 unchecked);
      assert(b1);
      //
      assert(sptr_delete->owned_by.size() == 0);
      assert(sptr_delete->owns.size() == 0);
      //
      // get its children
      auto children = std::move(sptr_delete->children);
      if (debug())
        std::cout << "destroy_pending: found |children|=" << children.size()
                  << std::endl;
      if (debug())
        std::cout << "destroy_pending: destroy node (move to vdata)"
                  << std::endl;
      if (debug()) {
        sptr_delete->debug_flag = true;
      }
      if (debug()) {
        std::cout << "CTX: will destroy EMPTY node: "
                  << sptr_delete->value_to_string() << std::endl;
      }
      // IMPORTANT: move data to vdata
      vdata.push_back(std::move(sptr_delete->value));
      // IMPORTANT: destroy node (without any data)
      sptr_delete = nullptr;
      //
      if (debug())
        std::cout << "destroy_pending: check children of node" << std::endl;
      // check if children can be saved
      while (children.size() > 0) {
        bool will_die = true;
        if (debug()) std::cout << "DEBUG: will move child!" << std::endl;
        auto sptr_child = std::move(children[0]);
        if (debug())
          std::cout << "DEBUG: child is " << sptr_child->value_to_string()
                    << std::endl;
        if (debug()) std::cout << "DEBUG: will erase empty child!" << std::endl;
        children.erase(children.begin() + 0);
        // I THINK THAT WE NEED TO CHECK isDescendent HERE BECAUSE MY CHILD
        // CANNOT OWN ME
        //
        if (unchecked) {
          // no solution for this child in UNCHECKED mode
          TNodeHelper<TNodeData>::cleanOwnsAndOwnedByLists(sptr_child, true);
        }

        for (unsigned k = 0; k < sptr_child->owned_by.size(); k++) {
          if (debug())
            std::cout << "DEBUG: child found new parent!" << std::endl;
          auto sptr_new_parent = sptr_child->owned_by[k].lock();
          //
          if (sptr_new_parent.get() == sptr_child.get()) {
            if (debug()) {
              std::cout << "Found new parent to own child but it's loop! "
                           "Ignoring! k="
                        << k << std::endl;
            }
            continue;
          }
          //
          if (debug())
            std::cout << "DEBUG: sptr_new_parent="
                      << sptr_new_parent->value_to_string() << std::endl;
          // NOTE: costly O(tree_size)=O(N) test in worst case for
          // 'isDescendent'
          bool _isDescendent =
              TNodeHelper<TNodeData>::isDescendent(sptr_new_parent, sptr_child);
          //
          if (debug())
            std::cout << "DEBUG: isDescendent=" << _isDescendent << " k=" << k
                      << std::endl;
          if (_isDescendent) {
            if (debug())
              std::cout
                  << "CTX DEBUG: owned_by is already my descendent! Discard. "
                  << "Will try next k!"
                  << "k=" << k << std::endl;
            // k++
            continue;
          }
          will_die = false;
          //
          sptr_child->parent = sptr_new_parent;
          TNodeHelper<TNodeData>::removeFromOwnsList(sptr_new_parent,
                                                     sptr_child);
          TNodeHelper<TNodeData>::removeFromOwnedByList(sptr_new_parent,
                                                        sptr_child);
          sptr_new_parent->add_child_strong(sptr_child);
        }
        // kill if not held by anyone now
        if (debug()) std::cout << "DEBUG: may kill child!" << std::endl;
        if (will_die) {
          if (debug())
            std::cout << "DEBUG: child will be send to pending list: "
                      << sptr_child->value_to_string() << std::endl;
          //
          // force clean both lists: owned_by and owns
          bool b1 =
              TNodeHelper<TNodeData>::cleanOwnsAndOwnedByLists(sptr_child);
          assert(b1);
          //
          pending.push_back(std::move(sptr_child));
          if (debug())
            std::cout << "DEBUG: child sent to pending list! |pending|="
                      << pending.size() << std::endl;
        } else {
          if (debug()) std::cout << "DEBUG: child is saved!" << std::endl;
          if (debug()) {
            std::cout << "CTX: child with these properties: ";
            std::cout << "node |owns|=" << sptr_child->owns.size()
                      << " |owned_by|=" << sptr_child->owned_by.size()
                      << std::endl;
          }
        }
      }  // while children exists
      //
    }  // while pending list > 0
    if (debug())
      std::cout << "destroy_pending: finished pending list |pending|="
                << pending.size() << std::endl;
    //
    if (debug())
      std::cout << "destroy_pending: final clear vdata. |vdata|="
                << vdata.size() << std::endl;
    vdata.clear();
    //
    if (debug())
      std::cout << "destroy_pending: assert no more pending. |pending|="
                << pending.size() << std::endl;
    // IF THIS FAILS, WE MAY NEED TO INTRODUCE ANOTHER WHILE LOOP HERE,
    // TO RESTART THE PROCESS, UNTIL WE FINISH WITH ZERO pending LIST.
    assert(pending.size() == 0);

    is_destroying = false;
    if (debug()) std::cout << "destroy_pending: finished!" << std::endl;
  }

 public:
  void print() override {
    std::cout << "print DynowForestV1: (forest size=" << forest.size() << ") ["
              << std::endl;
    for (const auto& p : forest) {
      std::cout << " ~> ROOT_NODE " << p.first << " as '" << (*p.first)
                << "' -> TREE " << p.second << ": ";
      p.second->print();
    }
    std::cout << "]" << std::endl;
  }
};

}  // namespace detail

}  // namespace cycles

#endif  // CYCLES_DynowForestV1_HPP_ // NOLINT