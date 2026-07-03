include_directories(
    .
    ${GST_INCLUDE_DIRS}
)

link_directories(
    ${GST_LIBRARY_DIRS}
)

add_executable(audio_manager

    main.c
    audio_manager.c
)

target_compile_options(audio_manager PRIVATE
    ${GST_CFLAGS_OTHER}
)

target_link_libraries(audio_manager

    ${GST_LIBRARIES}
)
