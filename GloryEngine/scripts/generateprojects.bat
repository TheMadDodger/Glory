@echo off

pushd ..\
call third-party\premake\premake5.exe vs2019
popd
PAUSE