cmake_minimum_required(VERSION 3.11...3.28)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(microMayaUSD VERSION 1.0
                     DESCRIPTION "A simple 3D model viewer and editor using Qt, integrating USD"
                     LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_RESOURCES
  resources/glsl.qrc
  forms/mainwindow.ui
  forms/cameracontrolshelp.ui
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS forms)
find_package(Qt6 COMPONENTS Core Widgets OpenGLWidgets REQUIRED)

find_package(pxr REQUIRED)
# Fix compilation error with C++17 on macos
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_STANDARD MATCHES "17")
    add_compile_definitions(_LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION)
endif()

include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
)

FetchContent_MakeAvailable(glm)

add_executable(microMayaUSD ${QT_RESOURCES})
add_subdirectory(src)
target_link_libraries(microMayaUSD PRIVATE
  Qt6::Core Qt6::Widgets Qt6::OpenGLWidgets
  glm::glm
  ${PXR_LIBRARIES}
)
target_include_directories(microMayaUSD PRIVATE
  ${PXR_INCLUDE_DIRS}
)