
set(SOURCE_FILES
    Server.cpp
    Client.cpp
)

add_library(chatserver SHARED ${SOURCE_FILES})
add_executable(run_chatserver main.cpp)

target_link_libraries(run_chatserver chatserver ${LIBS})

install(TARGETS chatserver DESTINATION lib)

