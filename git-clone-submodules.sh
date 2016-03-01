#!/bin/bash
deps="libuiohook PartialCsvParser PEGTL rapidjson";
for dep in $deps;
do 
	if [ "$(ls -A 3rdparty/github-InspectorWidget-$dep)" ]; then
		echo "$dep already cloned"
	else
		echo "cloning $dep"
		git clone --depth=1 https://github.com/InspectorWidget/$dep.git 3rdparty/github-InspectorWidget-$dep
	fi
done
