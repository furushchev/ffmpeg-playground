#!/bin/bash

docker run \
       --network=host \
       --device=/dev/dri:/dev/dri \
       -e ROS_MASTER_URI:$ROS_MASTER_URI \
       -e ROS_HOSTNAME:$ROS_HOSTNAME \
       -e ROS_IP:$ROS_IP \
       -e DISPLAY=$DISPLAY \
       -v /tmp/.X11-unix:/tmp/.X11-unix \
       --rm -it \
       ffmpeg-playground \
       $@
