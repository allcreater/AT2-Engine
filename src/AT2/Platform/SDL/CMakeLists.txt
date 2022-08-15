set (SDL_SOURCES_LIST
    "application.h"
    "application.cpp"
    "graphics_context.h"
    "graphics_context.cpp"
    "window.h"
    "window.cpp"
)

target_sources(${PROJECT_NAME} PRIVATE ${SDL_SOURCES_LIST})