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
    mkdir includes/curl

    echo "Building curl"
    cd curl
    mkdir Win32
    cd Win32
    cmake .. -A Win32
    cmake --build . --config debug
    cmake --build . --config release
    cd ..
    echo "Building x64 binaries"
    mkdir x64
    cd x64
    cmake .. -A x64
    cmake --build . --config debug
    cmake --build . --config release

    cd ../..

    echo "Building SDL"
    mkdir includes/Debug/Win32/SDL2
    mkdir includes/Debug/x64/SDL2
    mkdir includes/Release/Win32/SDL2
    mkdir includes/Release/x64/SDL2

    cd SDL
    echo "Building win32 binaries"
    mkdir win32
    cd Win32
    cmake .. -A Win32
    cmake --build . --config debug

    echo "Copying includes"
    find include -name \*.h -exec cp {} ../../includes/Debug/Win32/SDL2/ \;
    find include-config-debug -name \*.h -exec cp {} ../../includes/Debug/Win32/SDL2/ \;

    cmake --build . --config release

    echo "Copying includes"
    find include -name \*.h -exec cp {} ../../includes/Release/Win32/SDL2/ \;
    find include-config-release -name \*.h -exec cp {} ../../includes/Release/Win32/SDL2/ \;

    cd ..
    echo "Building x64 binaries"
    mkdir x64
    cd x64
    cmake .. -A x64
    cmake --build . --config debug
    find include -name \*.h -exec cp {} ../../includes/Debug/x64/SDL2/ \;
    find include-config-debug -name \*.h -exec cp {} ../../includes/Debug/x64/SDL2/ \;

    cmake --build . --config release
    find include -name \*.h -exec cp {} ../../includes/Release/x64/SDL2/ \;
    find include-config-release -name \*.h -exec cp {} ../../includes/Release/x64/SDL2/ \;

    cd ../..

    mkdir includes/SDL_image

    echo "Building SDL_image"
    cd SDL_image
    echo "Building win32 binaries"
    mkdir win32
    cd Win32
    cmake .. -A Win32 -DSDL2_INCLUDE_DIR=../../includes/Debug/Win32/SDL2 -DSDL2_LIBRARY=../../SDL/Win32/Debug/SDL2d.lib -DSDL2_MAIN_LIBRARY=../../SDL/Win32/Debug/SDL2maind.lib
    cmake --build . --config debug
    cmake .. -A Win32 -DSDL2_INCLUDE_DIR=../../includes/Release/Win32/SDL2 -DSDL2_LIBRARY=../../SDL/Win32/Release/SDL2.lib -DSDL2_MAIN_LIBRARY=../../SDL/Win32/Release/SDL2main.lib
    cmake --build . --config release

    cd ..
    echo "Building x64 binaries"
    mkdir x64
    cd x64

    cmake .. -A x64 -DSDL2_INCLUDE_DIR=../../includes/Debug/x64/SDL2 -DSDL2_LIBRARY=../../SDL/x64/Debug/SDL2d.lib -DSDL2_MAIN_LIBRARY=../../SDL/x64/Debug/SDL2maind.lib
    cmake --build . --config debug
    cmake .. -A x64 -DSDL2_INCLUDE_DIR=../../includes/Release/x64/SDL2 -DSDL2_LIBRARY=../../SDL/x64/Release/SDL2.lib -DSDL2_MAIN_LIBRARY=../../SDL/x64/Release/SDL2main.lib
    cmake --build . --config release

    cd ..
    cp SDL_image.h ../includes/SDL_image/SDL_image.h

    cd ..

    mkdir includes/curl

    echo "Building curl"
    cd curl

    echo "Building win32 binaries"
    mkdir Win32
    cd Win32
    cmake .. -A Win32 -DCURL_ENABLE_SSL=ON -DCURL_USE_OPENSSL=ON
    cmake --build . --config debug
    cmake --build . --config release
    cd lib
    cp curl_config.h ../../../includes/curl/curl_config.h
    cd ../..

    echo "Copying headers"
    find include/curl -name \*.h -exec cp {} ../includes/curl/ \;

    echo "Building x64 binaries"
    mkdir x64
    cd x64
    cmake .. -A x64 -DCURL_ENABLE_SSL=ON -DCURL_USE_OPENSSL=ON
    cmake --build . --config debug
    cmake --build . --config release

    cd ../..
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