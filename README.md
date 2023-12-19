# HyperCache


### Build Docker Images

To use a docker-image to build the project use:

```bash
# Build proxy
docker build -f docker/Dockerfile.hc_proxy.image_build -t hc_proxy .

# Build database
docker build -f docker/Dockerfile.hc_db.image_build -t hc_db .
```

To build from local source use:

```bash
# Build proxy
bazel build //hypercache_proxy:hc_proxy
docker build -f docker/Dockerfile.hc_proxy.local_build -t hc_proxy .

# Build database
bazel build //hypercache_db:hc_db
docker build -f docker/Dockerfile.hc_db.local_build -t hc_db .
```

**IMPORTANT**:
The Docker image installes the *libstdc++* on the container. To make sure the software runs stable, compile it with *gcc* or, when compiling with *clang*, change the Dockerfile to install *libc++* instead.


### Development


##### Completions / IntelliSense

For development I recommend using *clangd* for intellisense / documentation.
To generate the *compile_commands.json* file there are various options, I recommend to use this tool:

[bazel-compile-commands](https://github.com/kiron1/bazel-compile-commands)

[bazel-compile-commands releases](https://github.com/kiron1/bazel-compile-commands/releases)

Currently for me this is the most stable option.
