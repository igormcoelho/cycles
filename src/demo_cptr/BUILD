load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_import", "cc_library")

package(
    default_visibility = ["//visibility:public"],
)

cc_binary(
    name = "demo0_cycles_test",
    srcs = ["demo0_cycles_test_graph.cpp"],
    deps=["//include/cycles:cycles_hpp",
    "//include/demo_cptr:demo_cptr_hpp"]
)

cc_binary(
    name = "demo",
    srcs = ["demo.cpp"],
    deps=["//include/cycles:cycles_hpp",
    "//include/demo_cptr:demo_cptr_hpp"]
)
