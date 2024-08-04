#!/usr/bin/env bash

BASEDIR = "$(dirname "$(readlink -f "$0")")"
if [ "$1" == "--rebuild" ]; then rm -rf "$BASEDIR/build"; fi
mkdir -p $BASEDIR/build

cp $BASEDIR/docker-compose.yml $BASEDIR/build

docker build -t bleyn/ros-rtsp:latest $BASEDIR/..
if [ ! -e "$BASEDIR/build/ros-rtsp.tar.gz" ]; then
	docker image save bleyn/ros-rtsp:latest | gzip > $BASEDIR/build/ros-rtsp.tar.gz
fi

mkdir -p $BASEDIR/build/data
cp -r $BASEDIR/../launch $BASEDIR/build/data/launch

echo "!/usr/bin/env bash
BASEDIR = \"$(dirname \"\$(readlink -f \"\$0\")\")\"

docker load < \$BASEDIR/ros-rtsp.tar.gz
docker-compose -f \$BASEDIR/build/docker-compose.yml up -d" > $BASEDIR/build/deploy.sh