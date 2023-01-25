cd $(dirname "$0")
cd ..
clang-format -style=file -i src/*.cpp src/*.h commands/*/*.json

