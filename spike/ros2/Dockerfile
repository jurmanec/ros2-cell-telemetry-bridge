# Spike-only image: ROS 2 Humble + build tools. Not a production edge image.
FROM ros:humble-ros-base

RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-humble-rclcpp \
    ros-humble-std-msgs \
    python3-colcon-common-extensions \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /ws
COPY src ./src

SHELL ["/bin/bash", "-c"]
RUN source /opt/ros/humble/setup.bash \
    && colcon build --packages-select cell_telemetry_spike

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["ros2", "run", "cell_telemetry_spike", "cell_publisher"]
