cmake_minimum_required(VERSION 3.11)
project(MicroMayaUSD LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  src/camera.cpp
  src/cameracontrolshelp.cpp
  src/drawable.cpp
  src/la.cpp
  src/main.cpp
  src/mainwindow.cpp
  src/meshdata/face.cpp
  src/meshdata/halfedge.cpp
  src/meshdata/vertex.cpp
  src/mygl.cpp
  src/openglcontext.cpp
  src/scene/mesh.cpp
  src/scene/squareplane.cpp
  src/scene/wire/wireedge.cpp
  src/scene/wire/wireface.cpp
  src/scene/wire/wirevertex.cpp
  src/shaderprogram.cpp
  src/skeletondata/joint.cpp
  src/utils.cpp
)

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

# find_package(pxr REQUIRED)

include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
)

FetchContent_MakeAvailable(glm)

add_executable(${PROJECT_NAME} ${SOURCES} ${QT_RESOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE
  Qt6::Core Qt6::Widgets Qt6::OpenGLWidgets
  glm::glm
  # ${PXR_LIBRARIES}
)
target_include_directories(${PROJECT_NAME} PRIVATE
  src
  # ${PXR_INCLUDE_DIRS}
)