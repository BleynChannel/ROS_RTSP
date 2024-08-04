ARG ROS_DISTRO=noetic

FROM ros:$ROS_DISTRO

SHELL [ "/bin/bash", "-c" ]

# --- Install dependens ---
RUN apt-get update && \
	apt-get install -y --no-install-recommends ros-noetic-cv-bridge && \
	apt-get install -y --no-install-recommends ros-noetic-image-transport && \
	apt install -y --no-install-recommends ros-noetic-camera-info-manager && \
	apt install -y --no-install-recommends ros-noetic-tf && \
	apt-get install -y libavdevice-dev
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

WORKDIR /ros_ws
COPY ./app /ros_ws/src/ROS_RTSP

# --- Build ---	
RUN source /opt/ros/$ROS_DISTRO/setup.bash && \
	cd /ros_ws && catkin_make

# --- Configure ---
RUN echo "source /opt/ros/$ROS_DISTRO/setup.bash" >> ~/.bashrc && \
    echo "source /ros_ws/devel/setup.bash" >> ~/.bashrc

COPY ./ros_entrypoint.sh /
ENTRYPOINT ["/ros_entrypoint.sh"]

CMD ["roslaunch" "camera_driver" "TriCamera.launch"]