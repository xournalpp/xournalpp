if [ ! -d "build_docker/" ]; then
    mkdir "build_docker" 
fi
cd build_docker
cmake ..
cmake --build .
