workspace(name = "hypercache")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_github_nelhage_rules_boost",

    url = "https://github.com/nelhage/rules_boost/archive/ce2b65fd6d1494aadb2d8c99ce26aa222ab72486.tar.gz",
    strip_prefix = "rules_boost-ce2b65fd6d1494aadb2d8c99ce26aa222ab72486",
)
load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
boost_deps()
