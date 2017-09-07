cmake_minimum_required(VERSION 3.8)
project(SACA)

set(CMAKE_C_STANDARD 99)

set(COMMON_SRC
        common/net/message.c
        common/net/tcpcon.c
        common/net/tcpserv.c
        common/utils/base64.c
        common/utils/list.c
        common/utils/stringutils.c)

set(CONTROLLER_SRC
        ${COMMON_SRC}
        core/airplane.c
        core/collisiondetection.c
        core/controller.c)

set(PILOT_SRC
        ${COMMON_SRC}
        core/airplane.c
        core/pilot.c)

set(MON_SRC
        ${COMMON_SRC}
        core/airplane.c
        core/mon.c)

set(CMD_SRC
        ${COMMON_SRC}
        core/airplane.c
        core/cmd.c)

add_executable(controller ${CONTROLLER_SRC})
add_executable(pilot ${PILOT_SRC})
add_executable(mon ${MON_SRC})
add_executable(cmd ${CMD_SRC})

target_link_libraries(controller pthread m)
target_link_libraries(pilot pthread m)
target_link_libraries(mon pthread m)
target_link_libraries(cmd pthread m)
