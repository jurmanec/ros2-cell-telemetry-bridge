cmake_minimum_required(VERSION 3.8)
project(cell_bridge)

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

add_library(cell_bridge_batch src/batch_json.cpp)
target_include_directories(cell_bridge_batch PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(cell_bridge_batch cell_msgs)

add_executable(cell_bridge_node src/cell_bridge_node.cpp src/http_client.cpp)
target_include_directories(cell_bridge_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(cell_bridge_node cell_bridge_batch CURL::libcurl)
ament_target_dependencies(cell_bridge_node rclcpp cell_msgs)

install(TARGETS cell_bridge_node cell_bridge_batch
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_batch_json test/test_batch_json.cpp)
  target_link_libraries(test_batch_json cell_bridge_batch)
  ament_target_dependencies(test_batch_json cell_msgs)
endif()

ament_package()
