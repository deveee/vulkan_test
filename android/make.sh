#!/bin/sh
#
# (C) 2016-2017 Dawid Gan, under the GPLv3
#
# A script that creates the apk build


export DIRNAME=$(realpath "$(dirname "$0")")

export NDK_PATH_DEFAULT="$DIRNAME/android-ndk"
export SDK_PATH_DEFAULT="$DIRNAME/android-sdk"

export NDK_TOOLCHAIN_PATH="$DIRNAME/obj/bin"
export NDK_BUILD_SCRIPT="$DIRNAME/Android.mk"
export PATH="$DIRNAME/obj/bin:$PATH"
export CROSS_SYSROOT="$DIRNAME/obj/sysroot"

#export NDK_CCACHE=ccache
export NDK_CPPFLAGS="-O3 -g"

export NDK_ABI_ARMV7=armeabi-v7a
export ARCH_ARMV7=arm
export HOST_ARMV7=arm-linux-androideabi
export NDK_PLATFORM_ARMV7=android-26
export MIN_SDK_VERSION_ARMV7=26
export TARGET_SDK_VERSION_ARMV7=26
export COMPILE_SDK_VERSION_ARMV7=26

export NDK_ABI_AARCH64=arm64-v8a
export ARCH_AARCH64=arm64
export HOST_AARCH64=aarch64-linux-android
export NDK_PLATFORM_AARCH64=android-26
export MIN_SDK_VERSION_AARCH64=26
export TARGET_SDK_VERSION_AARCH64=26
export COMPILE_SDK_VERSION_AARCH64=26

export NDK_ABI_X86=x86
export ARCH_X86=x86
export HOST_X86=i686-linux-android
export NDK_PLATFORM_X86=android-26
export MIN_SDK_VERSION_X86=26
export TARGET_SDK_VERSION_X86=26
export COMPILE_SDK_VERSION_X86=26

export NDK_ABI_X86_64=x86_64
export ARCH_X86_64=x86_64
export HOST_X86_64=x86_64-linux-android
export NDK_PLATFORM_X86_64=android-26
export MIN_SDK_VERSION_X86_64=26
export TARGET_SDK_VERSION_X86_64=26
export COMPILE_SDK_VERSION_X86_64=26

export APP_NAME_RELEASE="Vulkan Test"
export PACKAGE_NAME_RELEASE="org.deve.vulkan_test"
export APP_DIR_NAME_RELEASE="vulkan-test"
export APP_ICON_RELEASE="$DIRNAME/icon.png"

export APP_NAME_BETA="Vulkan Test Beta"
export PACKAGE_NAME_BETA="org.deve.vulkan_test_beta"
export APP_DIR_NAME_BETA="vulkan-test-beta"
export APP_ICON_BETA="$DIRNAME/icon-dbg.png"

export APP_NAME_DEBUG="Vulkan Test Debug"
export PACKAGE_NAME_DEBUG="org.deve.vulkan_test_dbg"
export APP_DIR_NAME_DEBUG="vulkan-test-dbg"
export APP_ICON_DEBUG="$DIRNAME/icon-dbg.png"


# A helper function that checks if error ocurred
check_error()
{
    if [ $? -gt 0 ]; then
        echo "Error ocurred."
        exit
    fi
}

# Handle clean command
if [ ! -z "$1" ] && [ "$1" = "clean" ]; then
    rm -rf "$DIRNAME/bin"
    rm -rf "$DIRNAME/build"
    rm -rf "$DIRNAME/libs"
    rm -rf "$DIRNAME/obj"
    rm -rf "$DIRNAME/res"
    rm -rf "$DIRNAME/.gradle"
    exit
fi

# Check if compilation for different platform has been started before
if [ -f "$DIRNAME/obj/compile_arch" ]; then
    PROJECT_ARCH=$(cat "$DIRNAME/obj/compile_arch") 
    
    if [ -z "$COMPILE_ARCH" ]; then
        COMPILE_ARCH="$PROJECT_ARCH"
    elif [ "$PROJECT_ARCH" != "$COMPILE_ARCH" ]; then
        echo "Error: Compilation for different platform has been already made."
        echo "Run './make.sh clean' first or set COMPILE_ARCH variable" \
             "to '$PROJECT_ARCH.'"
        exit
    fi
fi

# Update variables for selected architecture
if [ -z "$COMPILE_ARCH" ]; then
    COMPILE_ARCH="armv7"
fi

if [ "$COMPILE_ARCH" = "armv7" ]; then
    export NDK_PLATFORM=$NDK_PLATFORM_ARMV7
    export NDK_ABI=$NDK_ABI_ARMV7
    export ARCH=$ARCH_ARMV7
    export HOST=$HOST_ARMV7
    export MIN_SDK_VERSION=$MIN_SDK_VERSION_ARMV7
    export TARGET_SDK_VERSION=$TARGET_SDK_VERSION_ARMV7
    export COMPILE_SDK_VERSION=$COMPILE_SDK_VERSION_ARMV7
elif [ "$COMPILE_ARCH" = "aarch64" ]; then
    export NDK_PLATFORM=$NDK_PLATFORM_AARCH64
    export NDK_ABI=$NDK_ABI_AARCH64
    export ARCH=$ARCH_AARCH64
    export HOST=$HOST_AARCH64
    export MIN_SDK_VERSION=$MIN_SDK_VERSION_AARCH64
    export TARGET_SDK_VERSION=$TARGET_SDK_VERSION_AARCH64
    export COMPILE_SDK_VERSION=$COMPILE_SDK_VERSION_AARCH64
elif [ "$COMPILE_ARCH" = "x86" ]; then
    export NDK_PLATFORM=$NDK_PLATFORM_X86
    export NDK_ABI=$NDK_ABI_X86
    export ARCH=$ARCH_X86
    export HOST=$HOST_X86
    export MIN_SDK_VERSION=$MIN_SDK_VERSION_X86
    export TARGET_SDK_VERSION=$TARGET_SDK_VERSION_X86
    export COMPILE_SDK_VERSION=$COMPILE_SDK_VERSION_X86
elif [ "$COMPILE_ARCH" = "x86_64" ]; then
    export NDK_PLATFORM=$NDK_PLATFORM_X86_64
    export NDK_ABI=$NDK_ABI_X86_64
    export ARCH=$ARCH_X86_64
    export HOST=$HOST_X86_64
    export MIN_SDK_VERSION=$MIN_SDK_VERSION_X86_64
    export TARGET_SDK_VERSION=$TARGET_SDK_VERSION_X86_64
    export COMPILE_SDK_VERSION=$COMPILE_SDK_VERSION_X86_64
else
    echo "Unknow COMPILE_ARCH: $COMPILE_ARCH. Possible values are: " \
         "armv7, aarch64, x86, x86_64"
    exit
fi

# Update variables for selected build type
if [ -z "$BUILD_TYPE" ]; then
    BUILD_TYPE="debug"
fi

if [ "$BUILD_TYPE" = "debug" ] || [ "$BUILD_TYPE" = "Debug" ]; then
    export ANT_BUILD_TYPE="debug"
    export GRADLE_BUILD_TYPE="assembleDebug"
    export IS_DEBUG_BUILD=1
    export APP_NAME="$APP_NAME_DEBUG"
    export PACKAGE_NAME="$PACKAGE_NAME_DEBUG"
    export APP_DIR_NAME="$APP_DIR_NAME_DEBUG"
    export APP_ICON="$APP_ICON_DEBUG"
elif [ "$BUILD_TYPE" = "release" ] || [ "$BUILD_TYPE" = "Release" ]; then
    export ANT_BUILD_TYPE="release"
    export GRADLE_BUILD_TYPE="assembleRelease"
    export IS_DEBUG_BUILD=0
    export APP_NAME="$APP_NAME_RELEASE"
    export PACKAGE_NAME="$PACKAGE_NAME_RELEASE"
    export APP_DIR_NAME="$APP_DIR_NAME_RELEASE"
    export APP_ICON="$APP_ICON_RELEASE"
elif [ "$BUILD_TYPE" = "beta" ] || [ "$BUILD_TYPE" = "Beta" ]; then
    export ANT_BUILD_TYPE="release"
    export GRADLE_BUILD_TYPE="assembleRelease"
    export IS_DEBUG_BUILD=0
    export APP_NAME="$APP_NAME_BETA"
    export PACKAGE_NAME="$PACKAGE_NAME_BETA"
    export APP_DIR_NAME="$APP_DIR_NAME_BETA"
    export APP_ICON="$APP_ICON_BETA"
else
    echo "Unsupported BUILD_TYPE: $BUILD_TYPE. Possible values are: " \
         "debug, release"
    exit
fi

# Check selected build tool
if [ -z "$BUILD_TOOL" ]; then
    BUILD_TOOL="gradle"
fi

if [ "$BUILD_TOOL" != "gradle" ] && [ "$BUILD_TOOL" != "ant" ]; then
    echo "Unsupported BUILD_TOOL: $BUILD_TOOL. Possible values are: " \
         "gradle, ant"
    exit
fi

# Check if we have access to the Android NDK and SDK
if [ -z "$NDK_PATH" ]; then
    export NDK_PATH="$NDK_PATH_DEFAULT"
fi

if [ -z "$SDK_PATH" ]; then
    export SDK_PATH="$SDK_PATH_DEFAULT"
fi

NDK_PATH=$(realpath "$NDK_PATH")
SDK_PATH=$(realpath "$SDK_PATH")

if [ ! -d "$NDK_PATH" ]; then
    echo "Error: Couldn't find $NDK_PATH directory. Please create a symlink" \
         "to your Android NDK installation in the $NDK_PATH_DEFAULT or set"  \
         "proper path in the NDK_PATH variable"
    exit
fi

if [ ! -d "$SDK_PATH" ]; then
    echo "Error: Couldn't find $SDK_PATH directory. Please create a symlink" \
         "to your Android SDK installation in the $SDK_PATH_DEFAULT or set"  \
         "proper path in the SDK_PATH variable"
    exit
fi

# Find newest build-tools version
if [ -z "$BUILD_TOOLS_VER" ]; then
    BUILD_TOOLS_DIRS=`ls -1 "$SDK_PATH/build-tools" | sort -V -r`
   
    for DIR in $BUILD_TOOLS_DIRS; do
        if [ "$DIR" = `echo $DIR | sed 's/[^0-9,.]//g'` ]; then
            BUILD_TOOLS_VER="$DIR"
            break
        fi
    done
fi

if [ -z "$BUILD_TOOLS_VER" ] || [ ! -d "$SDK_PATH/build-tools/$BUILD_TOOLS_VER" ]; then
    echo "Error: Couldn't detect build-tools version."
    exit
fi

# Set project version and code
if [ -f "$DIRNAME/obj/project_version" ]; then
    PROJECT_VERSION_PREV=$(cat "$DIRNAME/obj/project_version") 
    
    if [ -z "$PROJECT_VERSION" ]; then
        export PROJECT_VERSION="$PROJECT_VERSION_PREV"
    elif [ "$PROJECT_VERSION" != "$PROJECT_VERSION_PREV" ]; then
        echo "Different project version has been set. Forcing recompilation..."
        touch -c "$DIRNAME/Android.mk"
    fi
fi

if [ -z "$PROJECT_VERSION" ]; then
    if [ $IS_DEBUG_BUILD -ne 0 ]; then
        export PROJECT_VERSION="git"
    else
        echo "Error: Variable PROJECT_VERSION is not set. It must have unique" \
             "value for release build."
        exit
    fi
fi

if [ -z "$PROJECT_CODE" ]; then
    if [ $IS_DEBUG_BUILD -ne 0 ]; then
        PROJECT_CODE="1"
    else
        echo "Error: Variable PROJECT_CODE is not set."
        exit
    fi
fi

# Standalone toolchain
if [ ! -f "$DIRNAME/obj/make_standalone_toolchain.stamp" ]; then
    echo "Creating standalone toolchain"
    rm -rf "$DIRNAME/obj"
    ${NDK_PATH}/build/tools/make-standalone-toolchain.sh \
        --platform=$NDK_PLATFORM                         \
        --install-dir="$DIRNAME/obj/"                    \
        --arch=$ARCH
    check_error
    touch "$DIRNAME/obj/make_standalone_toolchain.stamp"
    echo $COMPILE_ARCH > "$DIRNAME/obj/compile_arch"
fi

echo "$PROJECT_VERSION" > "$DIRNAME/obj/project_version"

# Zlib
if [ ! -f "$DIRNAME/obj/zlib.stamp" ]; then
    echo "Compiling zlib"
    mkdir -p "$DIRNAME/obj/zlib"
    cp -a -f "$DIRNAME/../lib/zlib/"* "$DIRNAME/obj/zlib"

    cd "$DIRNAME/obj/zlib"
    cmake . -DCMAKE_C_FLAGS="-fPIC"                                       \
            -DCMAKE_CXX_FLAGS="-fPIC"                                     \
            -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
            -DHOST=$HOST &&
    make $@
    check_error
    touch "$DIRNAME/obj/zlib.stamp"
fi

# Libpng
if [ ! -f "$DIRNAME/obj/libpng.stamp" ]; then
    echo "Compiling libpng"
    mkdir -p "$DIRNAME/obj/libpng"
    mkdir -p "$DIRNAME/obj/libpng/lib"
    cp -a -f "$DIRNAME/../lib/libpng/"* "$DIRNAME/obj/libpng"

    cd "$DIRNAME/obj/libpng"
    cmake . -DCMAKE_TOOLCHAIN_FILE=../../../cmake/Toolchain-android.cmake \
            -DHOST=$HOST                                                  \
            -DZLIB_LIBRARY="$DIRNAME/obj/zlib/libz.a"                     \
            -DZLIB_INCLUDE_DIR="$DIRNAME/obj/zlib/"                       \
            -DCMAKE_CXX_FLAGS="-DPNG_ARM_NEON_OPT=0"                      \
            -DCMAKE_C_FLAGS="-DPNG_ARM_NEON_OPT=0"                        \
            -DPNG_TESTS=0 &&
    make $@
    check_error
    touch "$DIRNAME/obj/libpng.stamp"
fi

# Main
echo "Compiling main app"
cd "$DIRNAME"
${NDK_PATH}/ndk-build $@                 \
    APP_BUILD_SCRIPT="$NDK_BUILD_SCRIPT" \
    APP_ABI="$NDK_ABI"                   \
    APP_PLATFORM="$NDK_PLATFORM"         \
    APP_CPPFLAGS="$NDK_CPPFLAGS"         \
    APP_STL=c++_static                   \
    NDK_DEBUG=$IS_DEBUG_BUILD

check_error

# Build apk
echo "Building APK"

mkdir -p "$DIRNAME/res/drawable/"
mkdir -p "$DIRNAME/res/drawable-hdpi/"
mkdir -p "$DIRNAME/res/drawable-mdpi/"
mkdir -p "$DIRNAME/res/drawable-xhdpi/"
mkdir -p "$DIRNAME/res/drawable-xxhdpi/"
mkdir -p "$DIRNAME/res/values/"

STRINGS_FILE="$DIRNAME/res/values/strings.xml"

echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>"       >  "$STRINGS_FILE"
echo "<resources>"                                      >> "$STRINGS_FILE"
echo "    <string name=\"app_name\">$APP_NAME</string>" >> "$STRINGS_FILE"
echo "</resources>"                                     >> "$STRINGS_FILE"

sed -i "s/minSdkVersion=\".*\"/minSdkVersion=\"$MIN_SDK_VERSION\"/g" \
       "$DIRNAME/AndroidManifest.xml"
       
sed -i "s/targetSdkVersion=\".*\"/targetSdkVersion=\"$TARGET_SDK_VERSION\"/g" \
       "$DIRNAME/AndroidManifest.xml"
       
sed -i "s/package=\".*\"/package=\"$PACKAGE_NAME\"/g" \
       "$DIRNAME/AndroidManifest.xml"
       
sed -i "s/versionName=\".*\"/versionName=\"$PROJECT_VERSION\"/g" \
       "$DIRNAME/AndroidManifest.xml"
       
sed -i "s/versionCode=\".*\"/versionCode=\"$PROJECT_CODE\"/g" \
       "$DIRNAME/AndroidManifest.xml"
       
#cp "banner.png" "$DIRNAME/res/drawable/banner.png"
cp "$APP_ICON" "$DIRNAME/res/drawable/icon.png"
convert -scale 72x72 "$APP_ICON" "$DIRNAME/res/drawable-hdpi/icon.png"
convert -scale 48x48 "$APP_ICON" "$DIRNAME/res/drawable-mdpi/icon.png"
convert -scale 96x96 "$APP_ICON" "$DIRNAME/res/drawable-xhdpi/icon.png"
convert -scale 144x144 "$APP_ICON" "$DIRNAME/res/drawable-xxhdpi/icon.png"

if [ -f "/usr/lib/jvm/java-8-openjdk-amd64/bin/java" ]; then
    export JAVA_HOME="/usr/lib/jvm/java-8-openjdk-amd64"
    export PATH=$JAVA_HOME/bin:$PATH
fi

if [ "$BUILD_TOOL" = "gradle" ]; then
    export ANDROID_HOME="$SDK_PATH"
    gradle -Pcompile_sdk_version=$COMPILE_SDK_VERSION \
           -Pbuild_tools_ver="$BUILD_TOOLS_VER"       \
           $GRADLE_BUILD_TYPE
elif [ "$BUILD_TOOL" = "ant" ]; then
    ant -Dsdk.dir="$SDK_PATH"  \
        -Dtarget="android-$TARGET_SDK_VERSION" \
        $ANT_BUILD_TYPE
fi

check_error
