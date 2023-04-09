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

# ASSIMP
echo "Building ASSIMP"
cd assimp
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG

cd ../..
echo "Building SDL"

# SDL
cd SDL
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG

cd ../..

# SDL_image
echo "Building SDL_image"
cd SDL_image
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM -DSDL2_INCLUDE_DIR="../../includes/${CONFIG}/${PLATFORM}/SDL2" -DSDL2_LIBRARY="../../SDL/${PLATFORM}/${CONFIG}/${SDL_LIB}.lib" -DSDL2_MAIN_LIBRARY="../../SDL/${PLATFORM}/${CONFIG}/${SDL_MAIN_LIB}.lib" -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG

cd ../..

# SPIRV-tools
cd SPIRV-tools
echo "Cloning SPIRV-Headers"
git clone https://github.com/KhronosGroup/SPIRV-Headers.git external/spirv-headers

cd ..

# shaderc
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

# SPIRV-Cross
echo "Building SPIRV-Cross"
cd SPIRV-Cross
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG

cd ../..

# GLEW
echo "Building GLEW"
cd ../third-party
cd glew-2.2.0
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake ../build/cmake -A $PLATFORM -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG

cd ../../../submodules

echo "Building curl"
cd curl

#cp curl_config.h ../../../includes/curl/curl_config.h

#echo "Copying headers"
#find include/curl -name \*.h -exec cp {} ../includes/curl/ \;

mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM -DCMAKE_INSTALL_PREFIX=$DEPSDIR #-DCURL_ENABLE_SSL=ON -DCURL_USE_OPENSSL=ON
cmake --build . --target INSTALL --config $CONFIG

# Jolt
# Jolt now gets built in the jolt module itself because I could not solve an illusive linker error
#echo "Building Jolt Physics"
#cd JoltPhysics/Build
#echo "Building ${PLATFORM} binaries"
#mkdir "${PLATFORM}"
#cd "${PLATFORM}"
#cmake .. -A $PLATFORM -DCMAKE_INSTALL_PREFIX=$DEPSDIR -DUSE_STATIC_MSVC_RUNTIME_LIBRARY=OFF
#cmake --build . --target Jolt --config $CONFIG
#
#echo "Copying Lib to Depencencies"
#cp "${CONFIG}/Jolt.lib" "../../../../Dependencies/${CONFIG}/lib"
#cp "${CONFIG}/Jolt.pdb" "../../../../Dependencies/${CONFIG}/lib"
#
#echo "Copying Includes to Depencencies"
#cd ../..
#cp -r Jolt "../../Dependencies/${CONFIG}/include"
#find "../../Dependencies/${CONFIG}/include" -name \*.cpp -type f -delete