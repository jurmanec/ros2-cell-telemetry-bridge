# P3 edge image: ROS 2 Jazzy + cell_msgs + cell_sim + cell_bridge.
FROM ros:jazzy-ros-base

RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-jazzy-rclcpp \
    ros-jazzy-rosidl-default-generators \
    ros-jazzy-rosidl-default-runtime \
    ros-jazzy-builtin-interfaces \
    libcurl4-openssl-dev \
    python3-colcon-common-extensions \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /ws
COPY ws/src ./src

SHELL ["/bin/bash", "-c"]
RUN source /opt/ros/jazzy/setup.bash \
    && colcon build --packages-up-to cell_sim cell_bridge \
    && source install/setup.bash \
    && colcon test --packages-select cell_sim cell_bridge --event-handlers console_direct+ \
    && colcon test-result --verbose

COPY docker/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["ros2", "run", "cell_sim", "cell_publisher"]
