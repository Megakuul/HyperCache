conan install . --build=missing

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

cmake ..

if [ $? -eq 0 ]; then
	cmake --build .
fi

if [ $? -eq 0 ]; then
    ./hc_prox
else
    echo "Build failed. Please check the errors above."
fi
