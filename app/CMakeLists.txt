add_executable(lzip_app lzip_app.cpp)
set_target_properties(lzip_app PROPERTIES OUTPUT_NAME lzip)
target_link_libraries(lzip_app PRIVATE lzip)

add_custom_command(TARGET lzip_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
            $<TARGET_FILE:lzip_app>       # -> the real binary
            ${CMAKE_CURRENT_BINARY_DIR}/encode
    COMMAND ${CMAKE_COMMAND} -E create_symlink
            $<TARGET_FILE:lzip_app>
            ${CMAKE_CURRENT_BINARY_DIR}/decode
  )

set_property(DIRECTORY APPEND PROPERTY
  ADDITIONAL_CLEAN_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/encode"
    "${CMAKE_CURRENT_BINARY_DIR}/decode"
)
