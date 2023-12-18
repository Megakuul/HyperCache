set -e

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

conan install .. --build=missing

cmake .. -DCMAKE_TOOLCHAIN_FILE=./Release/generators/conan_toolchain.cmake

cmake --build .

./hc_proxy
