echo "$OSTYPE"

echo ${CONFIG=$1}

mkdir Dependencies
mkdir "Dependencies/${CONFIG}"
echo ${DEPSDIR="${PWD}/Dependencies/${CONFIG}"}

cd submodules

if [ "$OSTYPE" == "linux-gnu" ]; then
    # ...
    echo ""
elif [ "$OSTYPE" == "darwin" ]; then
    # Mac OSX
    echo ""
elif [ "$OSTYPE" == "cygwin" ]; then
    # POSIX compatibility layer and Linux environment emulation for Windows
    echo ""
elif [ "$OSTYPE" == "msys" ]; then
    echo "Windows system detected"
	echo ${PLATFORM=x64}
elif [ "$OSTYPE" == "win32" ]; then
    echo "32 bit windows is not supported"
    exit
elif [ "$OSTYPE" == "freebsd"* ]; then
    # ...
    echo ""
else
    # Unknown.
    echo ""
fi

echo "Building ASSIMP"
cd assimp
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG

cd ../..
echo "Building SDL"

cd SDL
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG

cd ../..

echo "Building SDL_image"
cd SDL_image
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM -DSDL2_INCLUDE_DIR="../../includes/${CONFIG}/${PLATFORM}/SDL2" -DSDL2_LIBRARY="../../SDL/${PLATFORM}/${CONFIG}/${SDL_LIB}.lib" -DSDL2_MAIN_LIBRARY="../../SDL/${PLATFORM}/${CONFIG}/${SDL_MAIN_LIB}.lib" -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG

cd ../..

cd SPIRV-tools
echo "Cloning SPIRV-Headers"
git clone https://github.com/KhronosGroup/SPIRV-Headers.git external/spirv-headers

cd ..
cd shaderc
echo "Building shaderc"
echo "Cloning glslang"
git clone https://github.com/KhronosGroup/glslang.git third_party/glslang

echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM -DSHADERC_SKIP_TESTS=ON -DSHADERC_SPIRV_TOOLS_DIR=../../SPIRV-tools -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG

cd ../..

echo "Building SPIRV-Cross"
cd SPIRV-Cross
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG

cd ../..

echo "Building GLEW"
cd ../third-party
cd glew-2.2.0
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake ../build/cmake -A $PLATFORM -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG