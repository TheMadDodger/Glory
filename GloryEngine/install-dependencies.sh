echo "$OSTYPE"

cd submodules
mkdir includes
mkdir includes/Debug
mkdir includes/Release
mkdir includes/Debug/Win32
mkdir includes/Release/Win32
mkdir includes/Debug/x64
mkdir includes/Release/x64

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

    ##echo "Building ASSIMP"
    ##cd submodules/assimp
    ##echo "Building win32 binaries"
    ##mkdir win32
    ##cd Win32
    ##cmake .. -A Win32
    ##cmake --build . --config debug
    ##cmake --build . --config release
    ##cd ..
    ##echo "Building x64 binaries"
    ##mkdir x64
    ##cd x64
    ##cmake .. -A x64
    ##cmake --build . --config debug
    ##cmake --build . --config release
    ##
    ##cd ../..

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

    #echo "Building SDL_image"
    #cd SDL_image
    #echo "Building win32 binaries"
    #mkdir win32
    #cd Win32
    #cmake .. -A Win32
    #cmake --build . --config debug
    #cmake --build . --config release
    #cd ..
    #echo "Building x64 binaries"
    #mkdir x64
    #cd x64
    #cmake .. -A x64
    #cmake --build . --config debug
    #cmake --build . --config release
elif [ "$OSTYPE" == "freebsd"* ]; then
    # ...
    echo ""
else
    # Unknown.
    echo ""
fi