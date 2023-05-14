
set(main_src
  breadcrumbsaddressbar.cpp
  breadcrumbsaddressbar.h
  breadcrumbslayout.cpp
  breadcrumbslayout.h
  filenamemodel.cpp
  filenamemodel.h
  menulistview.cpp
  menulistview.h
  styleproxy.cpp
  styleproxy.h
)

# Add files
file(GLOB_RECURSE UI_FILES *.ui)
file(GLOB_RECURSE RC_FILES *.rc)
file(GLOB_RECURSE QRC_FILES *.qrc)

# Filter
source_group("Ui Files" FILES ${UI_FILES})
source_group("Qrc Files" FILES ${QRC_FILES})

add_library(
  ${BREAD_CRUMBS_ADDRESS_BAR_LIB}
  ${main_src}
  ${UI_FILES}
  ${RC_FILES}
  ${QRC_FILES}
)

# Link
target_link_libraries(
  ${BREAD_CRUMBS_ADDRESS_BAR_LIB}
  Qt${QT_VERSION_MAJOR}::Widgets
)

target_include_directories(
  ${BREAD_CRUMBS_ADDRESS_BAR_LIB}
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})

