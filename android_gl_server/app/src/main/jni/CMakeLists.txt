# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.4.1)

set(GL_SERVER_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../gl_server)
#set(GL_SERVER_DIRS /work/codes/gl-streaming/gl_server)
set(GL_COMMON_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../common)

add_definitions(-D__ANDROID__)
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
include_directories(
        ${GL_SERVER_DIRS}
        ${GL_COMMON_DIRS}
)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
find_library( # Sets the name of the path variable.
        log_lib
        android
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log
        )

find_library( # Sets the name of the path variable.
        graphic_lib
        EGL
        GLESv2
        )

add_library( # Sets the name of the library.
        gl_server
        # Sets the library as a shared library.
        STATIC
        # Provides a relative path to your source file(s).
        ${GL_COMMON_DIRS}/fifo.c
        ${GL_COMMON_DIRS}/server.c
        ${GL_SERVER_DIRS}/glcontrol.c
        ${GL_SERVER_DIRS}/glserver.c
        ${GL_SERVER_DIRS}/serveregl.c
        ${GL_SERVER_DIRS}/servergles.c
        ${GL_SERVER_DIRS}/main.c
        )

target_link_libraries( # Specifies the target library.
        gl_server
        # Links the target library to the log library
        # included in the NDK.
        GLESv2
        log
        )

add_library( # Sets the name of the library.
        glstreamserver
        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        glstreaming_jni.cpp
        )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        glstreamserver
        # Links the target library to the log library
        gl_server
        # included in the NDK.
        ${log_lib}
        ${graphic_lib}

        )


