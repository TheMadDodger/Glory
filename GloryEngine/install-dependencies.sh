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
elif [ "$OSTYPE" == "msys" ] || [ "$OSTYPE" == "win32" ]; then
    echo "Windows system detected"
	echo ${PLATFORM=Win32}

elif [ "$OSTYPE" == "freebsd"* ]; then
    # ...
    echo ""
else
    # Unknown.
    echo ""
fi

mkdir "includes/${CONFIG}/${PLATFORM}"

echo "Building ASSIMP"
cd assimp
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM
cmake --build . --config $CONFIG

cd ../..

echo "Building SDL"
mkdir "includes/${CONFIG}/${PLATFORM}/SDL2"

cd SDL
echo "Building ${PLATFORM} binaries"
mkdir "${PLATFORM}"
cd "${PLATFORM}"
cmake .. -A $PLATFORM
cmake --build . --config $CONFIG

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
cmake .. -A $PLATFORM -DSDL2_INCLUDE_DIR="../../includes/${CONFIG}/${PLATFORM}/SDL2" -DSDL2_LIBRARY="../../SDL/${PLATFORM}/${CONFIG}/SDL2.lib" -DSDL2_MAIN_LIBRARY="../../SDL/${PLATFORM}/${CONFIG}/SDL2main.lib"
cmake --build . --config $CONFIG

cd ..
echo "Copying includes"
cp SDL_image.h ../includes/SDL_image/SDL_image.h