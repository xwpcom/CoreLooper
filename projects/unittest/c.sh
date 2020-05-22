#!/bin/bash
#XiongWanPing 2018.06.24

#默认配置
builtType="debug"   # can be debug,release
_BOARD="linux"		# can be linux,hi3516

#编译默认配置
#./c.sh debug
#./c.sh release

#编译指定平台
#./c.sh debug hi3516
#./c.sh release linux


set -x

if [ $# -gt 0 ]; then

	if [ "$1" = "debug" ]; then
		builtType="debug"
	else
		if [ "$1" = "release" ]; then
			builtType="release"
		else
			_BOARD="$1"
		fi
	fi
fi

	
if [ $# -gt 1 ]; then
	_BOARD="$2"
fi


SOURCE_DIR=`pwd`

if [ "$_BOARD" = "hi3516" ]; then
	export CC=/opt/hisi-linux/x86-arm/arm-hisiv300-linux/bin/arm-hisiv300-linux-uclibcgnueabi-gcc
	export CXX=/opt/hisi-linux/x86-arm/arm-hisiv300-linux/bin/arm-hisiv300-linux-uclibcgnueabi-g++
	BUILD_DIR=${BUILD_DIR:-./build.hi3516}
else
	BUILD_DIR=${BUILD_DIR:-./build.linux}
fi

BUILD_TYPE=${BUILD_TYPE:-"$builtType"}

mkdir -p $BUILD_DIR/$BUILD_TYPE \
  && cd $BUILD_DIR/$BUILD_TYPE \
  && cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE $SOURCE_DIR \
  && make 

