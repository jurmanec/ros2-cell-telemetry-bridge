cmake_minimum_required(VERSION 3.8)
project(cell_historian)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cell_msgs REQUIRED)
find_package(CURL REQUIRED)

add_library(cell_historian_line src/weld_line.cpp)
target_include_directories(cell_historian_line PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(cell_historian_line cell_msgs)

add_executable(weld_historian_node src/weld_historian_node.cpp src/influx_write.cpp)
target_include_directories(weld_historian_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(weld_historian_node cell_historian_line CURL::libcurl)
ament_target_dependencies(weld_historian_node rclcpp cell_msgs)

install(TARGETS weld_historian_node cell_historian_line
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_weld_line test/test_weld_line.cpp)
  target_link_libraries(test_weld_line cell_historian_line)
  ament_target_dependencies(test_weld_line cell_msgs)
endif()

ament_package()
