FROM ros:jazzy-ros-base

RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-jazzy-rclcpp \
    ros-jazzy-rosidl-default-generators \
    ros-jazzy-rosidl-default-runtime \
    ros-jazzy-builtin-interfaces \
    libcurl4-openssl-dev \
    python3-colcon-common-extensions \
    clangd \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspaces/edge

SHELL ["/bin/bash", "-c"]
RUN echo 'source /opt/ros/jazzy/setup.bash' >> /root/.bashrc \
 && echo '[ -f /workspaces/edge/ws/install/setup.bash ] && source /workspaces/edge/ws/install/setup.bash' >> /root/.bashrc
