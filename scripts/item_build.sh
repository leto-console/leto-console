ROOT=$(pwd)
cd $1
cmake --preset $2 -DLETO_PATH="$ROOT"
cmake --build -j --preset $2
