cmake_minimum_required(VERSION 3.25)
project(modding_suite)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/modules/")
message(${CMAKE_MODULE_PATH})

find_package(GLFW3 REQUIRED)
find_package(Gettext REQUIRED)
find_package(Epoxy REQUIRED)
set(Python_FIND_VIRTUALENV ONLY)
find_package(Python COMPONENTS Interpreter Development)
find_package(pybind11 CONFIG REQUIRED HINTS /ucrt64/lib/cmake/pybind11 /ucrt64/include /ucrt64)

set(VENV ${CMAKE_BINARY_DIR}/venv)
set(PYDIR ${VENV}/bin)
execute_process(COMMAND_ECHO STDOUT COMMAND ${Python_EXECUTABLE} -m venv ${VENV})
execute_process(COMMAND_ECHO STDOUT COMMAND ${PYDIR}/python -m pip install wgrd-cons-parsers wgrd-cons-tools)

add_subdirectory(deps/wgrd-tools/)
add_subdirectory(deps/magic_enum)

add_library(imgui
  deps/imgui/imgui.cpp
  deps/imgui/imgui_tables.cpp
  deps/imgui/imgui_draw.cpp
  deps/imgui/imgui_widgets.cpp
  deps/imgui/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(imgui PUBLIC 
deps/imgui/
deps/imgui/misc/cpp/
)

add_executable(modding_suite
		ImGuiFileDialog/ImGuiFileDialog.cpp
		src/main.cpp
		src/imgui_impl_glfw.cpp
		src/imgui_impl_opengl3.cpp
		src/file_tree.cpp
		src/file_tree.h
		src/wgrd_files.cpp
		src/wgrd_files.h
    src/maingui.cpp
    src/maingui.hpp
    src/ndftransactions.cpp
    src/ndftransactions.hpp
    src/workspace.cpp
    src/workspace.hpp
)

target_link_libraries(modding_suite
	PRIVATE
  ${GLFW3_LIBRARY}
	pybind11::embed
	${LibEpoxy_LIBRARIES}
  imgui
  ndf
  magic_enum::magic_enum
  $<$<BOOL:${WIN32}>:intl iconv kernel32 opengl32 glu32 -static>
)

target_include_directories(modding_suite
PRIVATE
deps/wgrd-tools/src
ImGuiFileDialog/
${LibEpoxy_INCLUDE_DIRS}
)

install(TARGETS modding_suite DESTINATION bin)
include(CPack)
