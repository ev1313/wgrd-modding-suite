cmake_minimum_required(VERSION 3.25)
project(modding_suite)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/modules/")
message(${CMAKE_MODULE_PATH})

find_package(GLFW3 REQUIRED)
find_package(Gettext REQUIRED)
find_package(Epoxy REQUIRED)
find_package(Python3 3.11 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

set(VENV ${CMAKE_BINARY_DIR}/venv)
set(PYDIR ${VENV}/bin)

if(${LINUX})
execute_process(COMMAND_ECHO STDOUT COMMAND ${Python_EXECUTABLE} -m venv ${VENV})
execute_process(COMMAND_ECHO STDOUT COMMAND ${PYDIR}/python -m pip install wgrd-cons-parsers wgrd-cons-tools)
endif()

add_subdirectory(deps/wgrd-tools/)
add_subdirectory(deps/magic_enum)
add_subdirectory(deps/toml11)

# imgui dependecies
add_library(imgui
  deps/imgui/imgui.cpp
  deps/imgui/imgui_tables.cpp
  deps/imgui/imgui_draw.cpp
  deps/imgui/imgui_widgets.cpp
  deps/imgui/misc/cpp/imgui_stdlib.cpp
  deps/imgui/backends/imgui_impl_glfw.cpp
  deps/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
deps/imgui/
deps/imgui/misc/cpp/
deps/imgui/backends/
)
target_link_libraries(imgui PUBLIC ${GLFW3_LIBRARY} ${LibEpoxy_LIBRARIES})

add_library(imgui_filedialog STATIC
    ImGuiFileDialog/ImGuiFileDialog.cpp
)
target_include_directories(imgui_filedialog PUBLIC
    ImGuiFileDialog/
)
target_link_libraries(imgui_filedialog PUBLIC imgui)

# modding-suite

add_library(lib_modding_suite STATIC
    src/file_tree.cpp
	src/file_tree.hpp

    src/files/configs.hpp
    src/files/file.hpp
    src/files/file.cpp
    src/files/files.hpp
    src/files/files.cpp
    src/files/dic.hpp
    src/files/dic.cpp
    src/files/edat.hpp
    src/files/edat.cpp
    src/files/ess.hpp
    src/files/ess.cpp
    src/files/ndfbin.hpp
    src/files/ndfbin.cpp
    src/files/ppk.hpp
    src/files/ppk.cpp
    src/files/scenario.hpp
    src/files/scenario.cpp
    src/files/sformat.hpp
    src/files/sformat.cpp
    src/files/tgv.hpp
    src/files/tgv.cpp
    src/ndftransactions.cpp
    src/ndftransactions.hpp
    src/workspace.cpp
    src/workspace.hpp
    src/helpers.hpp
    src/imgui_helpers.hpp
    src/imgui_helpers.cpp
)
target_link_libraries(lib_modding_suite
    PUBLIC
    imgui_filedialog
    Python3::Python
    pybind11::pybind11
    ndf
    magic_enum::magic_enum
    toml11::toml11
    $<$<BOOL:${WIN32}>:intl iconv>
)
target_include_directories(lib_modding_suite
    PUBLIC
    deps/wgrd-tools/src
    src/
)

# modding-suite executable

add_executable(modding_suite
	src/main.cpp
    src/maingui.cpp
    src/maingui.hpp
)

target_link_libraries(modding_suite
	PRIVATE
	${LibEpoxy_LIBRARIES}
	lib_modding_suite
)

target_include_directories(modding_suite
PRIVATE
${LibEpoxy_INCLUDE_DIRS}
)

option(BUILD_TESTS "Build tests" OFF)
if(${BUILD_TESTS})
add_subdirectory(deps/Catch2)

# tests

add_executable(tests
    tests/file_tree.cpp
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain lib_modding_suite)

add_executable(tests_generator
    tests/generator.cpp
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain lib_modding_suite)
endif()

install(TARGETS modding_suite DESTINATION bin)
include(CPack)
