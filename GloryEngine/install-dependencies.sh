echo "$OSTYPE"

cd submodules/assimp

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

    echo "Building win32 binaries"
    mkdir win32
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
elif [ "$OSTYPE" == "freebsd"* ]; then
    # ...
    echo ""
else
    # Unknown.
    echo ""
fi