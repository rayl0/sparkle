@echo off

mkdir build
pushd build
cl -Zi ../code/win32_sparkle.cpp User32.lib gdi32.lib
popd build