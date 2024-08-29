cd $(dirname "$0")
cd ..
clang-format -style=llvm -i cli/*.cpp  include/*.h python/*.cpp src/*.cpp tests/*.cpp js/*.cpp
