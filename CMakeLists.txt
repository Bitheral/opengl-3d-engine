cmake_minimum_required(VERSION 3.17)

project(3DEngine)

add_executable(
        ${PROJECT_NAME}
        lib/CoreStructures/AABB.cpp
        lib/CoreStructures/AABB.h
        lib/CoreStructures/Camera.cpp
        lib/CoreStructures/Camera.h
        lib/CoreStructures/Circle.cpp
        lib/CoreStructures/Circle.h
        lib/CoreStructures/FloatImage.cpp
        lib/CoreStructures/FloatImage.h
        lib/CoreStructures/Mesh.cpp
        lib/CoreStructures/Mesh.h
        lib/CoreStructures/Model.cpp
        lib/CoreStructures/Model.h
        lib/CoreStructures/Noise.h
        lib/CoreStructures/NoiseImages.cpp
        lib/CoreStructures/NoiseImages.h
        lib/CoreStructures/PerlinNoise.cpp
        lib/CoreStructures/PerlinNoise.h
        lib/CoreStructures/PrincipleAxes.cpp
        lib/CoreStructures/PrincipleAxes.h
        lib/CoreStructures/ScreenAlignedQuad.cpp
        lib/CoreStructures/ScreenAlignedQuad.h
        lib/CoreStructures/ShaderLoader.cpp
        lib/CoreStructures/ShaderLoader.h
        lib/CoreStructures/SkinnedMesh.cpp
        lib/CoreStructures/SkinnedMesh.h
        lib/CoreStructures/SkyBox.cpp
        lib/CoreStructures/SkyBox.h
        lib/CoreStructures/Sphere.cpp
        lib/CoreStructures/Sphere.h
#        lib/CoreStructures/TextRenderer.cpp
#        lib/CoreStructures/TextRenderer.h
        lib/CoreStructures/TexturedQuad.cpp
        lib/CoreStructures/TexturedQuad.h
        lib/CoreStructures/TextureLoader.cpp
        lib/CoreStructures/TextureLoader.h
        lib/CoreStructures/Timer.cpp
        lib/CoreStructures/Timer.h
        glad.c
        Includes.h
        main.cpp
)

target_link_libraries(${PROJECT_NAME} png)

include_directories("lib/CoreStructures")

find_package(assimp REQUIRED)
target_link_libraries(${PROJECT_NAME} assimp)

find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} glfw)

find_package(freetype REQUIRED)
target_link_libraries(${PROJECT_NAME} freetype)