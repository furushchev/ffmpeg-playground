#!/bin/bash

docker run \
       --network=host \
       --device=/dev/dri:/dev/dri \
       -e ROS_MASTER_URI:$ROS_MASTER_URI \
       -e ROS_HOSTNAME:$ROS_HOSTNAME \
       -e ROS_IP:$ROS_IP \
       --rm -it \
       gitaiinc/gitai_image_transport:$ROS_DISTRO \
       $@
