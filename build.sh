# rm -rf ./out/build/release
rm -rf ./out/build/debug
# mkdir -p out/build/release/
mkdir -p out/build/debug/
# cmake -S . -B ./out/build/release -GNinja -DCMAKE_CXX_COMPILER="/bin/clang++-12" -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_PARALLEL_LEVEL=3
# cmake --build ./out/build/release --config Release
# cmake -S . -B ./out/build/debug -GNinja -DCMAKE_CXX_COMPILER="/bin/clang++" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_BUILD_PARALLEL_LEVEL=3
cmake -S . -B ./out/build/debug -DCMAKE_CXX_COMPILER="/bin/clang++" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_BUILD_PARALLEL_LEVEL=3
# cmake --build ./out/build/debug --config Debug
