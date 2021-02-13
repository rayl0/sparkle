@echo off

mkdir build
pushd build
cl /FC -Zi ../code/win32_sparkle.cpp User32.lib gdi32.lib
popd build
