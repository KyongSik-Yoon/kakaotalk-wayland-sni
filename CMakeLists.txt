cmake_minimum_required(VERSION 3.20)

project(kakaotalk-wayland-sni VERSION 0.1.0 LANGUAGES CXX)

include(CTest)
include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core DBus)

add_library(kakaotalk-sni-core STATIC
    src/actions.cpp
    src/config.cpp
    src/process_detector.cpp
)
target_include_directories(kakaotalk-sni-core PUBLIC src)
target_link_libraries(kakaotalk-sni-core PUBLIC Qt6::Core)
target_compile_options(kakaotalk-sni-core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(kakaotalk-wayland-sni
    src/main.cpp
    src/status_notifier.cpp
)
target_link_libraries(kakaotalk-wayland-sni
    PRIVATE
        kakaotalk-sni-core
        Qt6::Core
        Qt6::DBus
)
target_compile_options(kakaotalk-wayland-sni PRIVATE -Wall -Wextra -Wpedantic)

install(TARGETS kakaotalk-wayland-sni RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

configure_file(
    packaging/kakaotalk-wayland-sni.service.in
    kakaotalk-wayland-sni.service
    @ONLY
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kakaotalk-wayland-sni.service
    DESTINATION ${CMAKE_INSTALL_DATADIR}/systemd/user
)

if(BUILD_TESTING)
    add_executable(kakaotalk-wayland-sni-tests
        tests/core_tests.cpp
    )
    target_link_libraries(kakaotalk-wayland-sni-tests PRIVATE kakaotalk-sni-core Qt6::Core)
    target_compile_options(kakaotalk-wayland-sni-tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME core-tests COMMAND kakaotalk-wayland-sni-tests)
endif()
