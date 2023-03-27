
set(main_src
  main.cpp
  output_thumbnail.h
  output_thumbnail.cpp
  MFUtility.h
  dxhelper.h
  sprite.h
  sprite.cpp
)

add_executable(${PROJECT_NAME} ${main_src})

target_link_libraries(${PROJECT_NAME}
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::GUI Qt${QT_VERSION_MAJOR}::Widgets)
