cmake_minimum_required(VERSION 3.25)
project(modding_suite)

set(CMAKE_CXX_STANDARD 23)

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

add_executable(modding_suite
		ImGuiFileDialog/ImGuiFileDialog.cpp
		src/main.cpp
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
    src/helpers.hpp
    src/imgui_helpers.hpp
    src/imgui_helpers.cpp
)

target_link_libraries(modding_suite
	PRIVATE
  $<$<BOOL:${WIN32}>:intl iconv>
  Python3::Python
	pybind11::pybind11
	${LibEpoxy_LIBRARIES}
  imgui
  ndf
  magic_enum::magic_enum
  toml11::toml11
)

target_include_directories(modding_suite
PRIVATE
deps/wgrd-tools/src
ImGuiFileDialog/
${LibEpoxy_INCLUDE_DIRS}
)

install(TARGETS modding_suite DESTINATION bin)
include(CPack)
