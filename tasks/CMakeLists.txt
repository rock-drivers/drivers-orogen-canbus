# Generated from orogen/lib/orogen/templates/tasks/CMakeLists.txt

include(canbusTaskLib)
ADD_LIBRARY(${CANBUS_TASKLIB_NAME} SHARED 
    ${CANBUS_TASKLIB_SOURCES})

add_dependencies(${CANBUS_TASKLIB_NAME}
    regen-typekit)


TARGET_LINK_LIBRARIES(${CANBUS_TASKLIB_NAME}
    ${OrocosRTT_LIBRARIES}
    ${CANBUS_TASKLIB_DEPENDENT_LIBRARIES})
SET_TARGET_PROPERTIES(${CANBUS_TASKLIB_NAME}
    PROPERTIES LINK_INTERFACE_LIBRARIES "${CANBUS_TASKLIB_INTERFACE_LIBRARIES}")

INSTALL(TARGETS ${CANBUS_TASKLIB_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib/orocos)

INSTALL(FILES ${CANBUS_TASKLIB_HEADERS}
    DESTINATION include/orocos/canbus)

