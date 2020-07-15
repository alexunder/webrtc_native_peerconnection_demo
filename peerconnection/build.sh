
set -eu

readonly ROOT_PATH=$(cd $(dirname $0) && pwd)

echo ${ROOT_PATH}

readonly LOCAL_ENV_PATH=${ROOT_PATH}/local

export CXX=g++

readonly BUILD_PATH=${ROOT_PATH}/build
mkdir -p ${BUILD_PATH}

cd ${BUILD_PATH}
cmake -DLIBWEBRTC_PATH=${LOCAL_ENV_PATH} ..
make

cp webrtc_client ${ROOT_PATH}
cp webrtc_server ${ROOT_PATH}
