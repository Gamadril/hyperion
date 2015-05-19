#!/bin/bash

# Make sure /sbin is on the path (for service to find sub scripts)
PATH="/sbin:$PATH"

# Script for downloading and installing the latest Hyperion release

# Find out if we are on Raspbmc
IS_XBIAN=$(cat /etc/issue | grep XBian | wc -l)
IS_RASPBMC=$(cat /etc/issue | grep Raspbmc | wc -l)
IS_OPENELEC=$(cat /etc/issue | grep -m 1 OpenELEC | wc -l)

# Find out if its an imx6 device
IS_IMX6=$(cat /proc/cpuinfo | grep i.MX6 | wc -l)
IS_RPI=$(cat /proc/cpuinfo | grep BCM27 | wc -l)

# detect x86 cpu 
TARGET_TYPE=$(uname -m)

if [ $IS_OPENELEC == 1 ]; then
	TARGET_PATH="/storage"
else
	TARGET_PATH="/opt"
fi

# check which init script we should use
USE_INITCTL=$(which /sbin/initctl | wc -l)
USE_SERVICE=$(which /usr/sbin/service | wc -l)

# Make sure that the boblight daemon is no longer running
BOBLIGHT_PROCNR=$(pidof boblightd | wc -l)
if [ $BOBLIGHT_PROCNR == 1 ];
then
	echo "Found running instance of boblight. Please stop boblight via XBMC menu before installing hyperion"
	exit
fi

# Stop hyperion daemon if it is running
# Start the hyperion daemon
if [ $USE_INITCTL == 1 ]; then
	/sbin/initctl stop hyperion
elif [ $USE_SERVICE == 1 ]; then
	/usr/sbin/service hyperion stop
fi

# Get and extract the Hyperion binaries and effects
echo "Downloading hyperion"
if [ $TARGET_TYPE == "x86_64" ]; then
	# x86_64
	curl -L --get https://raw.githubusercontent.com/Gamadril/hyperion/master/deploy/hyperion_x86_64.tar.gz | tar -C $TARGET_PATH -xz
elif [ $TARGET_TYPE == "i686" ]; then
	# i686
	curl -L --get https://raw.githubusercontent.com/Gamadril/hyperion/master/deploy/hyperion_i686.tar.gz | tar -C $TARGET_PATH -xz
	echo ""
elif [ $IS_IMX6 == 1 ]; then
	# imx6
	curl -L --get https://raw.githubusercontent.com/Gamadril/hyperion/master/deploy/hyperion_imx6.tar.gz | tar -C $TARGET_PATH -xz
else
	# rpi
	curl -L --get https://raw.githubusercontent.com/Gamadril/hyperion/master/deploy/hyperion.tar.gz | tar -C $TARGET_PATH -xz
fi

if [ $IS_OPENELEC == 1 ]; then
	# OpenELEC has a readonly file system. Use alternative location
	echo "Downloading missing openelec libraries"
	
	if [ $TARGET_TYPE == "x86_64" ]; then
		# Openelec 64bit for x86
		curl -L --get https://raw.githubusercontent.com/Gamadril/hyperion/master/deploy/hyperion.deps.openelec_x86_64.tar.gz | tar -C $TARGET_PATH/hyperion/bin -xz
	elif [ $TARGET_TYPE == "i686" ]; then
		# Openelec 32bit for i686
		curl -L --get https://raw.githubusercontent.com/Gamadril/hyperion/master/deploy/hyperion.deps.openelec_i686.tar.gz | tar -C $TARGET_PATH/hyperion/bin -xz
	elif [ $IS_IMX6 == 1 ]; then
		# Openelec for imx6
		curl -L --get https://raw.githubusercontent.com/Gamadril/hyperion/master/deploy/hyperion.deps.openelec_imx6.tar.gz | tar -C $TARGET_PATH/hyperion/bin -xz
	else
		# Openelec for rpi
		curl -L --get https://raw.githubusercontent.com/Gamadril/hyperion/master/deploy/hyperion.deps.openelec-rpi.tar.gz | tar -C $TARGET_PATH/hyperion/bin -xz
	fi
	
	# modify the default config to have a correct effect path
	sed -i "s:/opt:${TARGET_PATH}:g" $TARGET_PATH/hyperion/config/hyperion.config.json
	
	# copy to alternate location, because of readonly file system
	# /storage/.config is available as samba share. A symbolic link would not be working
	false | cp -i $TARGET_PATH/hyperion/config/hyperion.config.json $TARGET_PATH/.config/hyperion.config.json 2>/dev/null
else
	# create links to the binaries	
	ln -fs $TARGET_PATH/hyperion/bin/hyperiond /usr/bin/hyperiond
	ln -fs $TARGET_PATH/hyperion/bin/hyperion-remote /usr/bin/hyperion-remote
	ln -fs $TARGET_PATH/hyperion/bin/hyperion-v4l2 /usr/bin/hyperion-v4l2
	
	# create link to the gpio changer (gpio->spi) for rpi
	if [ $IS_RASPBMC == 1 ] && [ $IS_RPI == 1 ]; then
		ln -fs $TARGET_PATH/hyperion/bin/gpio2spi /usr/bin/gpio2spi
	fi
	
	# Copy a link to the hyperion configuration file to /etc
	ln -s $TARGET_PATH/hyperion/config/hyperion.config.json /etc/hyperion.config.json
fi

# Copy the service control configuration to /etc/int
if [ $USE_INITCTL == 1 ]; then
	echo "Installing initctl script"
	if [ $IS_RPI == 1 ]; then
		if [ $IS_RASPBMC == 1 ]; then
			wget -N https://raw.githubusercontent.com/Gamadril/hyperion/master/deploy/hyperion.conf -P /etc/init/
		else
			wget -N https://raw.githubusercontent.com/Gamadril/hyperion/master/deploy/hyperion.xbian.conf -O /etc/init/hyperion.conf
		fi
	fi
elif [ $USE_SERVICE == 1 ]; then
	echo "Installing startup script in init.d"
	# place startup script in init.d and add it to upstart
	ln -fs $TARGET_PATH/hyperion/init.d/hyperion.init.sh /etc/init.d/hyperion
	chmod +x /etc/init.d/hyperion
	update-rc.d hyperion defaults 98 02
elif [ $IS_OPENELEC == 1 ]; then
	# only add to start script if hyperion is not present yet
	if [ $(cat /storage/.config/autostart.sh 2>/dev/null | grep hyperiond | wc -l) == 0 ]; then
		echo "Adding Hyperion to autostart script"
		echo "$TARGET_PATH/hyperion/bin/hyperiond.sh $TARGET_PATH/.config/hyperion.config.json > /dev/null 2>&1 &" >> $TARGET_PATH/.config/autostart.sh
		chmod +x /storage/.config/autostart.sh
	fi
fi

# Start the hyperion daemon
if [ $USE_INITCTL == 1 ]; then
	/sbin/initctl start hyperion
elif [ $USE_SERVICE == 1 ]; then
	/usr/sbin/service hyperion start
fi
