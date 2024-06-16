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
rm "${PLATFORM}" -r
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG

cd ../..
echo "Building SDL"

# SDL
cd SDL
echo "Building ${PLATFORM} binaries"
rm "${PLATFORM}" -r
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG

if [ "$CONFIG" == "Debug" ]; then
    echo ${SDL_LIB="SDL2d"}
    echo ${SDL_MAIN_LIB="SDL2maind"}
else
    echo ${SDL_LIB="SDL2"}
    echo ${SDL_MAIN_LIB="SDL2main"}
fi

cd ../..

# SDL_image
echo "Building SDL_image"
cd SDL_image
echo "Building ${PLATFORM} binaries"
rm "${PLATFORM}" -r
mkdir "${PLATFORM}"
cd "${PLATFORM}"

cmake .. -A $PLATFORM -DSDL2_INCLUDE_DIR="../../includes/${CONFIG}/${PLATFORM}/SDL2" -DSDL2_LIBRARY="../../SDL/${PLATFORM}/${CONFIG}/${SDL_LIB}.lib" -DSDL2_MAIN_LIBRARY="../../SDL/${PLATFORM}/${CONFIG}/${SDL_MAIN_LIB}.lib" -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG

cd ../..

# SDL_mixer
echo "Building SDL_mixer"
cd SDL_mixer
echo "Building ${PLATFORM} binaries"
rm "${PLATFORM}" -r
mkdir "${PLATFORM}"
cd "${PLATFORM}"

cmake .. -A $PLATFORM -DSDL2_INCLUDE_DIR="../../includes/${CONFIG}/${PLATFORM}/SDL2" -DSDL2_LIBRARY="../../SDL/${PLATFORM}/${CONFIG}/${SDL_LIB}.lib" -DSDL2_MAIN_LIBRARY="../../SDL/${PLATFORM}/${CONFIG}/${SDL_MAIN_LIB}.lib" -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG

cd ../..

# shaderc
cd shaderc
echo "Building shaderc"
echo "Syncing depencencies"

python ./utils/git-sync-deps

echo "Building ${PLATFORM} binaries"
rm "${PLATFORM}" -r
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM -DSHADERC_SKIP_TESTS=ON -DCMAKE_INSTALL_PREFIX=$DEPSDIR -DSHADERC_ENABLE_SHARED_CRT=ON
cmake --build . --target INSTALL --config $CONFIG

cd ../..

# SPIRV-Cross
echo "Building SPIRV-Cross"
cd SPIRV-Cross
echo "Building ${PLATFORM} binaries"
rm "${PLATFORM}" -r
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
rm "${PLATFORM}" -r
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake ../build/cmake -A $PLATFORM -DCMAKE_INSTALL_PREFIX=$DEPSDIR
cmake --build . --target INSTALL --config $CONFIG

cd ../../../submodules

echo "Building curl"
cd curl

cp curl_config.h ../../../includes/curl/curl_config.h

echo "Copying headers"
find include/curl -name \*.h -exec cp {} ../includes/curl/ \;

rm "${PLATFORM}" -r
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM -DCMAKE_INSTALL_PREFIX=$DEPSDIR #-DCURL_ENABLE_SSL=ON -DCURL_USE_OPENSSL=ON
cmake --build . --target INSTALL --config $CONFIG
cd ../..

#efsw
echo "Building efsw"
cd efsw
../../third-party/premake/premake5.exe vs2022
cd make/windows
msbuild.exe -p:Configuration=$CONFIG -p:Platform=x64 efsw.sln
cd ../..

if [ "$CONFIG" == "Debug" ]; then
    echo ${EFSW_LIB="efsw-static-debug"}
else
    echo ${EFSW_LIB="efsw-static-release"}
fi

cp "lib/${EFSW_LIB}.lib" "../../Dependencies/${CONFIG}/lib/${EFSW_LIB}.lib"
cp "lib/${EFSW_LIB}.idb" "../../Dependencies/${CONFIG}/lib/${EFSW_LIB}.idb"
cp "lib/${EFSW_LIB}.pdb" "../../Dependencies/${CONFIG}/lib/${EFSW_LIB}.pdb"

mkdir "../../Dependencies/${CONFIG}/include/efsw"
cp "include/efsw/efsw.h" "../../Dependencies/${CONFIG}/include/efsw/efsw.h"
cp "include/efsw/efsw.hpp" "../../Dependencies/${CONFIG}/include/efsw/efsw.hpp"
cd ..

cd mono
msbuild.exe msvc/mono.sln -p:Platform=x64 -p:Configuration=$CONFIG -p:MONO_TARGET_GC=sgen
find "msvc/build/sgen/x64/lib/${CONFIG}" -name \*.* -exec cp {} "../../Dependencies/${CONFIG}/lib/" \;
find "msvc/build/sgen/x64/bin/${CONFIG}" -name \*.* -exec cp {} "../../Dependencies/${CONFIG}/bin/" \;
find "msvc/build/sgen/x64/bin/${CONFIG}" -name \*.* -exec cp {} "../../Dependencies/${CONFIG}/bin/" \;
cp -r msvc/include/mono "../../Dependencies/${CONFIG}/include"

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