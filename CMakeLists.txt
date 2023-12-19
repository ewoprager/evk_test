cmake_minimum_required(VERSION 3.24)

file(
GLOB_RECURSE HEADERS
RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"include/*.hpp"
)

file(
GLOB_RECURSE SOURCES
RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"src/*.cpp"
)

file(
GLOB_RECURSE UNPROCESSED_SHADERS
RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"UnprocessedShaders/*"
)

file(
GLOB_RECURSE UNPROCESSED_RESOURCES
RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"UnprocessedResources/*"
)

project(evk_test CXX)

add_executable(${PROJECT_NAME}
            "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCES}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${HEADERS}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${UNPROCESSED_RESOURCES}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${UNPROCESSED_SHADERS}"
            )

file(COPY "Resources/"
     DESTINATION "Resources/")

target_include_directories(${PROJECT_NAME} PUBLIC
                           "${CMAKE_CURRENT_SOURCE_DIR}/include/"
						   "/usr/local/include/"
                           "/Users/eprager/local/include/"
                           "/Users/eprager/local/include/mat4x4/"
                           "/Users/eprager/local/include/ESDL/"
                           "/Users/eprager/local/include/evk/"
                           "/Users/eprager/local/include/vulkan/"
                           "/opt/local/include/SDL2/"
                           "/opt/local/include/"
                           )

target_link_directories(${PROJECT_NAME} PUBLIC
                        "/usr/local/lib/"
						"/Users/eprager/local/lib/"
                        "/opt/local/lib/"
                        )

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_link_libraries(${PROJECT_NAME}
                      ESDL
                      SDL2
                      SDL2_image
                      vulkan
                      evk
                      mat4x4
                      )
