call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64

if not exist build mkdir build
cd build
cmake -G"Visual Studio 17 2022" ..
cmake --build . --config Release
cd ..
pause