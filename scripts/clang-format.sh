cd $(dirname "$0")
cd ..
clang-format -style=file -i src/*.cpp src/*.h
clang-format -i commands/*/*.json -style=gnu
