mkdir build_docs
cd build_docs
# Only build docs (for CI/CD)
echo "INFO: Build Doxygen...";
cmake -G Ninja ..
ninja doc