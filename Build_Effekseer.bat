mkdir Effekseer_Build
mkdir Effekseer
cd Effekseer_Build
cmake -G "Visual Studio 16" -A x64 -DCMAKE_DEBUG_POSTFIX=d -DCMAKE_INSTALL_PREFIX=../Effekseer -D BUILD_EXAMPLES=OFF -D BUILD_DX12=OFF ../../Effekseer
cmake --build . --config Debug --target INSTALL
cmake --build . --config Release --target INSTALL
