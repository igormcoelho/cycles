load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_import", "cc_library")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "cycles_hpp",
    hdrs = glob([
        "**/*.hpp",
    ]),
    include_prefix = "cycles/",
)
