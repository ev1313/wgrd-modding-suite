cmake_minimum_required(VERSION 3.25)
project(modding_suite)

set(CMAKE_CXX_STANDARD 23)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(imgui REQUIRED)
set(Python_FIND_VIRTUALENV ONLY)
find_package(Python COMPONENTS Interpreter Development)
find_package(pybind11 CONFIG)
find_package(imgui REQUIRED)
find_package(argparse REQUIRED)

set(VENV ${CMAKE_BINARY_DIR}/venv)
set(PYDIR ${VENV}/bin)
execute_process(COMMAND_ECHO STDOUT COMMAND ${Python_EXECUTABLE} -m venv ${VENV})
execute_process(COMMAND_ECHO STDOUT COMMAND ${PYDIR}/python -m pip install wgrd-cons-parsers wgrd-cons-tools)

add_subdirectory(deps/wgrd-tools/)

add_executable(modding_suite
		ImGuiFileDialog/ImGuiFileDialog.cpp
		main.cpp
		imgui_impl_glfw.cpp
		imgui_impl_opengl3.cpp
		file_tree.cpp
		file_tree.h
		wgrd_files.cpp
		wgrd_files.h
    maingui.cpp
    maingui.hpp
)

target_link_libraries(modding_suite
	PRIVATE
	pybind11::embed
	glfw
	OpenGL::GL
	imgui::imgui
	argparse::argparse
  etceterapp
)

target_include_directories(modding_suite
PRIVATE
deps/wgrd-tools/src
)

install(TARGETS modding_suite DESTINATION bin)
include(CPack)
