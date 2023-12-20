cc_binary(
    name = "hypercache_proxy",
    srcs = glob(["src/*.cc"]) + glob(["include/*.h"]),
    copts = ["-Ihypercache_proxy/include"],
    deps = ["@boost//:asio_ssl"],
)
