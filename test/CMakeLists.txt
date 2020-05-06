set (LIB_NAME realtinet)

IF(UNIX)

    add_executable(tcp_chat_client tcp_chat_client.cpp)
    target_link_libraries(tcp_chat_client ${LIB_NAME})

    add_executable(tcp_chat_server tcp_chat_server.cpp)
    target_link_libraries(tcp_chat_server ${LIB_NAME})

    add_executable(udp_chat_client udp_chat_client.cpp)
    target_link_libraries(udp_chat_client ${LIB_NAME})

    add_executable(udp_chat_server udp_chat_server.cpp)
    target_link_libraries(udp_chat_server ${LIB_NAME})

# else()
#   	message(SEND_ERROR "You are on an unsupported platform! (Not Win32 or Unix)")
ENDIF()