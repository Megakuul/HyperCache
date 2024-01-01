cc_binary(
	name = "hypercache_db",
	srcs = glob(["src/*.cc"]) + glob(["include/*.h"]),
	copts = ["-Ihypercache_db/include", "-std=c++23"],
	deps = ["@boost//:asio_ssl", "//lib/hypermap:hypermap"],
)
