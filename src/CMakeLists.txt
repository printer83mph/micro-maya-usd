target_sources(microMayaUSD PRIVATE
  camera.h
  camera.cpp
  cameracontrolshelp.h
  cameracontrolshelp.cpp
  drawable.h
  drawable.cpp
  la.h
  la.cpp
  main.cpp
  mainwindow.h
  mainwindow.cpp
  mygl.h
  mygl.cpp
  openglcontext.h
  openglcontext.cpp
  shaderprogram.h
  shaderprogram.cpp
  utils.h
  utils.cpp
)

target_include_directories(microMayaUSD PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(meshdata)
add_subdirectory(scene)
add_subdirectory(skeletondata)
