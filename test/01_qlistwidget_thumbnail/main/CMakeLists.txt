
set(main_src
  main.cpp
  mainwindow.h
  mainwindow.cpp
)

add_executable(
  ${PROJECT_NAME}
  WIN32
  ${main_src}
  ${UI_FILES}
  ${RC_FILES}
  ${QRC_FILES}
)

# Link
target_link_libraries(
  ${PROJECT_NAME}
  Qt5::Widgets
)

