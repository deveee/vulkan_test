LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# zlib
LOCAL_MODULE := zlib
LOCAL_SRC_FILES := obj/zlib/libz.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)

# PNG
LOCAL_MODULE := png
LOCAL_SRC_FILES := obj/libpng/libpng.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)

# Main
LOCAL_MODULE       := main
LOCAL_PATH         := .
LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_SRC_FILES    := $(wildcard ../src/*.cpp)     \
                      $(wildcard ../src/*/*.cpp)   \
                      $(wildcard ../src/*/*/*.cpp)
LOCAL_LDLIBS       := -llog -landroid -lvulkan
LOCAL_CFLAGS       := -I../src               \
                      -I../lib/glm           \
                      -Iobj/libpng/          \
                      -Iobj/zlib/            \
                      -I$(call my-dir)/../../sources/android/native_app_glue \
                      -DNDEBUG               \
                      -std=gnu++0x

LOCAL_STATIC_LIBRARIES := png zlib c++_static \
                          android_native_app_glue

include $(BUILD_SHARED_LIBRARY)
include $(CLEAR_VARS)

$(call import-module,android/native_app_glue)
