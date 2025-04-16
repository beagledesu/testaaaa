cmake_minimum_required(VERSION 3.16)
project(apex_original_overlay VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 出力ディレクトリの設定
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# vcpkgでインストールしたライブラリのパスを指定
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
else()
    set(CMAKE_TOOLCHAIN_FILE "C:/TEMP/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# 必要なパッケージの検索
find_package(ixwebsocket CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# インクルードディレクトリ
include_directories(${PROJECT_SOURCE_DIR}/include)

# ソースファイル
file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

# 実行ファイル
add_executable(apex_overlay ${SOURCES})

# ライブラリのリンク
target_link_libraries(apex_overlay PRIVATE 
    ixwebsocket::ixwebsocket
    nlohmann_json::nlohmann_json
)

# Windowsの場合
if(WIN32)
    target_link_libraries(apex_overlay PRIVATE ws2_32 wsock32)
endif()

# Webコンテンツのコピー
add_custom_command(TARGET apex_overlay POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/web $<TARGET_FILE_DIR:apex_overlay>/web
)

# 設定ファイルのコピー
add_custom_command(TARGET apex_overlay POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/config $<TARGET_FILE_DIR:apex_overlay>/config
)