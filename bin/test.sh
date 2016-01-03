#!/bin/bash
TARGET_PATH="/MY_PATH"

sed -i "s:/opt:${TARGET_PATH}:g" ./hyperion.config.json
