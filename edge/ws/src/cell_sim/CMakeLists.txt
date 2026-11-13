cmake_minimum_required(VERSION 3.8)
project(cell_sim)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cell_msgs REQUIRED)

add_executable(cell_publisher src/cell_publisher.cpp)
target_include_directories(cell_publisher PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(cell_publisher rclcpp cell_msgs)

add_executable(weld_cell_publisher src/weld_cell_publisher.cpp)
target_include_directories(weld_cell_publisher PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(weld_cell_publisher rclcpp cell_msgs)

add_executable(facility_asset_publisher src/facility_asset_publisher.cpp)
target_include_directories(facility_asset_publisher PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(facility_asset_publisher rclcpp cell_msgs)

install(TARGETS cell_publisher weld_cell_publisher facility_asset_publisher DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_telemetry_sample test/test_telemetry_sample.cpp)
  target_include_directories(test_telemetry_sample PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
  ament_target_dependencies(test_telemetry_sample rclcpp cell_msgs)

  ament_add_gtest(test_weld_snapshot test/test_weld_snapshot.cpp)
  target_include_directories(test_weld_snapshot PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
  ament_target_dependencies(test_weld_snapshot rclcpp cell_msgs)

  ament_add_gtest(test_facility_snapshot test/test_facility_snapshot.cpp)
  target_include_directories(test_facility_snapshot PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
  ament_target_dependencies(test_facility_snapshot rclcpp cell_msgs)
endif()

ament_package()
