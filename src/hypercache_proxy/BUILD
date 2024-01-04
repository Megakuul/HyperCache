cc_binary(
    name = "hypercache_proxy",
    srcs = glob(["src/*.cc"]) + glob(["include/*.h"]),
    copts = ["-Ihypercache_proxy/include", "-std=c++23"],
    deps = ["@boost//:asio_ssl"],
)
