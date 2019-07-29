FROM gitaiinc/openvisualcloud-ubuntu1604-media-ffmpeg:1.2

#COPY .docker.ssh /root/.ssh
#ENV GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no"

# Install ROS
RUN apt-get update -qq -y && apt-get install -qq -y lsb-release apt-transport-https python-pip python-numpy git

RUN echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list
RUN apt-key adv --keyserver 'hkp://keyserver.ubuntu.com:80' --recv-key C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654
#RUN apt-get update -qq && apt-get install ros-kinetic-ros-base -qq -y
RUN apt-get update -qq && apt-get install ros-kinetic-ros-base ros-kinetic-cv-bridge ros-kinetic-image-transport -qq -y

# Build package
COPY . /project
RUN mkdir -p /project/build
WORKDIR /project/build
RUN . /opt/ros/kinetic/setup.sh && \
    cmake .. && \
    make -j2

# setup entrypoint
CMD ["bash"]
