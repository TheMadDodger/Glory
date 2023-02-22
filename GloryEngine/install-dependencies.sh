echo "$OSTYPE"

echo ${CONFIG=$1}

cd submodules
mkdir includes
mkdir "includes/${CONFIG}"

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

mkdir "includes/${CONFIG}/${PLATFORM}"

mkdir includes/assimp
mkdir includes/assimp/Compiler
mkdir includes/assimp/port
mkdir includes/assimp/port/AndroidJNI

echo "Building ASSIMP"
cd assimp
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM
cmake --build . --config $CONFIG

echo "Copying includes"
find "include/assimp" -name \config.h -exec cp {} "../../includes/assimp/" \;
cd ..
find include -name \*.h -exec cp {} "../includes/assimp/" \;
find include/assimp/Compiler -name \*.h -exec cp {} "../includes/assimp/Compiler/" \;
find include/assimp/port/AndroidJNI -name \*.h -exec cp {} "../includes/assimp/port/AndroidJNI/" \;
find include -name \*.hpp -exec cp {} "../includes/assimp/" \;
find include -name \*.inl -exec cp {} "../includes/assimp/" \;
find include -name \*.in -exec cp {} "../includes/assimp/" \;
cd ..

echo "Building SDL"
mkdir "includes/${CONFIG}/${PLATFORM}/SDL2"

cd SDL
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM
cmake --build . --config $CONFIG

if [ "${CONFIG}" == "Release" ]; then
    echo ${SDL_LIB=SDL2}
    echo ${SDL_MAIN_LIB=SDL2main}
elif [ "${CONFIG}" == "Debug" ]; then
    echo ${SDL_LIB=SDL2d}
    echo ${SDL_MAIN_LIB=SDL2maind}
fi

echo "Copying includes"
find include -name \*.h -exec cp {} "../../includes/${CONFIG}/${PLATFORM}/SDL2/" \;
find "include-config-${CONFIG}" -name \*.h -exec cp {} "../../includes/${CONFIG}/${PLATFORM}/SDL2/" \;

cd ../..

mkdir includes/SDL_image

echo "Building SDL_image"
cd SDL_image
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM -DSDL2_INCLUDE_DIR="../../includes/${CONFIG}/${PLATFORM}/SDL2" -DSDL2_LIBRARY="../../SDL/${PLATFORM}/${CONFIG}/${SDL_LIB}.lib" -DSDL2_MAIN_LIBRARY="../../SDL/${PLATFORM}/${CONFIG}/${SDL_MAIN_LIB}.lib"
cmake --build . --config $CONFIG

cd ..
echo "Copying includes"
cp SDL_image.h ../includes/SDL_image/SDL_image.h
cd ..

cd SPIRV-tools
echo "Building SPIRV-Tools"
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
cmake .. -A $PLATFORM -DSHADERC_SKIP_TESTS=ON -DSHADERC_SPIRV_TOOLS_DIR=../../SPIRV-tools
cmake --build . --config $CONFIG

cd ../..

echo "Building SPIRV-Cross"
cd SPIRV-Cross
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM
cmake --build . --config $CONFIG

cd ../..

echo "Building GLEW"
cd ../third-party
cd glew-2.2.0
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake ../build/cmake -A $PLATFORM
cmake --build . --config $CONFIG