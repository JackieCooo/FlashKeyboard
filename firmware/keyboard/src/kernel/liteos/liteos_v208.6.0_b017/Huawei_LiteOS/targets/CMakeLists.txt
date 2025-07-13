
# add board special configuration #
if(NOT ${LOSCFG_FAMILY} STREQUAL "")
    set(FAMILY_CMAKE_PATH ${LITEOSTOPDIR}/targets/${LOSCFG_FAMILY}/family.cmake)
    if(EXISTS ${FAMILY_CMAKE_PATH})
        include(${FAMILY_CMAKE_PATH})
    endif()
else()
    set(BOARD_CMAKE_PATH ${LITEOSTOPDIR}/targets/${LITEOS_PLATFORM}/board.cmake)
    if(EXISTS ${BOARD_CMAKE_PATH})
        include(${BOARD_CMAKE_PATH})
    endif()
endif()

