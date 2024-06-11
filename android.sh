export ANDROID_HOME='/c/Users/alexs/AppData/Local/Android/Sdk'
export ANDROID_SDK_ROOT='/c/Users/alexs/AppData/Local/Android/Sdk'
export ANDROID_NDK_ROOT='/c/Program Files/Android/android-ndk-r26d'
export PATH=$PATH:'/c/Program Files/Android/cmdline-tools/bin'

export OPENSSL_INCLUDE_DIR='/msys64/mingw64/include/openssl'

mkdir android-build || echo 'android-build already exists'
cd android-build || exit 1

echo $PATH

/c/msys64/mingw64/bin/cmake .. -DCMAKE_TOOLCHAIN_FILE=../android-toolchain.cmake -DCMAKE_PREFIX_PATH='/msys64/mingw64/lib/cmake'
