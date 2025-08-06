set(COMPONENT_NAME "main_app")

file(GLOB BLE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/ble_service/*.c)
file(GLOB PERI_SRC ${CMAKE_CURRENT_SOURCE_DIR}/peripheral/*.c)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main_app.c
    ${BLE_SRC}
    ${PERI_SRC}
)

set(PRIVATE_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/peripheral
    ${CMAKE_CURRENT_SOURCE_DIR}/ble_service
)

set(PUBLIC_HEADER
)

set(PRIVATE_DEFINES
)

set(PUBLIC_DEFINES
)

set(COMPONENT_PUBLIC_CCFLAGS
)

set(COMPONENT_CCFLAGS
)

set(WHOLE_LINK
true
)

set(BUILD_AS_OBJ
false
)

set(MAIN_COMPONENT
true
)

build_component()
