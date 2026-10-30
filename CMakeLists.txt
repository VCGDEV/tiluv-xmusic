cmake_minimum_required(VERSION 3.15)
project(tiluv-xmusic VERSION 1.0.0 LANGUAGES CXX)

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

add_executable(tiluv-xmusic ${SOURCES})

target_include_directories(tiluv-xmusic PRIVATE
    include
    ${libvlc_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}
)

target_link_libraries(tiluv-xmusic
    PRIVATE
    SDL2::SDL2
    SDL2_ttf::SDL2_ttf
    ${libvlc_LIBRARIES}
)

# PNG/JPG cover art needs SDL2_image; without it only BMP images load.
if(SDL2_image_FOUND)
    target_link_libraries(tiluv-xmusic PRIVATE SDL2_image::SDL2_image)
    target_compile_definitions(tiluv-xmusic PRIVATE TILUV_HAS_SDL_IMAGE)
else()
    message(WARNING "SDL2_image not found: cover art limited to BMP. On Fedora: sudo dnf install SDL2_image-devel")
endif()

target_compile_options(tiluv-xmusic PRIVATE -Wall -Wextra)

# Screen resolution of the ILI9341 panel, and where skins/fonts/images live.
# On the Orange Pi, point TILUV_ASSET_DIR at the installed assets folder.
set(TILUV_ASSET_DIR "${CMAKE_SOURCE_DIR}/assets" CACHE PATH "Directory containing skins, fonts and images")
target_compile_definitions(tiluv-xmusic PRIVATE
    SCREEN_WIDTH=240
    SCREEN_HEIGHT=320
    TILUV_ASSET_DIR="${TILUV_ASSET_DIR}"
)
