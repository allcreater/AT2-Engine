set(GLFW_SOURCES_LIST
    "glfw_application.h"
    "glfw_application.cpp"
    "glfw_window.h"
    "glfw_window.cpp"
)

target_sources(${PROJECT_NAME} PRIVATE ${GLFW_SOURCES_LIST})