cmake_minimum_required(VERSION 3.15)
project(TalivXMusic VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image QUIET)
find_package(PkgConfig REQUIRED)

pkg_check_modules(libvlc REQUIRED libvlc)
pkg_check_modules(nlohmann_json REQUIRED nlohmann_json)

file(GLOB SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(taluv_xmusic ${SOURCES})

target_include_directories(taluv_xmusic PRIVATE
    include
    ${libvlc_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}
)

target_link_libraries(taluv_xmusic
    PRIVATE
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
    ${libvlc_LIBRARIES}
)

# PNG/JPG cover art needs SDL2_image; without it only BMP images load.
if(SDL2_image_FOUND)
    target_link_libraries(taluv_xmusic PRIVATE SDL2_image::SDL2_image)
    target_compile_definitions(taluv_xmusic PRIVATE TALUV_HAS_SDL_IMAGE)
else()
    message(WARNING "SDL2_image not found: cover art limited to BMP. On Fedora: sudo dnf install SDL2_image-devel")
endif()

target_compile_options(taluv_xmusic PRIVATE -Wall -Wextra)

# Screen resolution of the ILI9341 panel, and where skins/fonts/images live.
# On the Orange Pi, point TALUV_ASSET_DIR at the installed assets folder.
set(TALUV_ASSET_DIR "${CMAKE_SOURCE_DIR}/assets" CACHE PATH "Directory containing skins, fonts and images")
target_compile_definitions(taluv_xmusic PRIVATE
    SCREEN_WIDTH=240
    SCREEN_HEIGHT=320
    TALUV_ASSET_DIR="${TALUV_ASSET_DIR}"
)
