# InspectorWidget packaging system.

#=============================================================================
# Copyright 2015 Christian Frisson / UMONS.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

# at the end of any application that needs packaging, adapt the following variables:
# * SET(PROGNAME "app-name") where app-name matches the CMake target name of the application
# * (optional) SET(WITH_QT ON) if you application uses Qt4 as GUI framework
# * (optional) SET(MC_PACKAGE_DESCRIPTION "...")
# then add after: INCLUDE (${CMAKE_SOURCE_DIR}/cmake/CreatePackage.cmake)
#
# to create the package (with app-name the CMake target name of the application):
# Apple *.app bundle: make app-name install
# Linux *.deb package: cpack --config CPackConfig-app-name.cmake

# PACKAGING WITH CPack
# For more see http://www.cmake.org/Wiki/CMake:Packaging_With_CPack
IF(NOT USE_DEBUG) # mandatory for packaging release versions

IF(UNIX OR APPLE) # not yet tested with Windows 
	INCLUDE(InstallRequiredSystemLibraries)
	set(CPACK_PACKAGE_NAME "${PROGNAME}")
	set(CPACK_BUNDLE_NAME "${PROGNAME}")
    IF(NOT CPACK_PACKAGE_DESCRIPTION_SUMMARY AND NOT DESCRIPTION)
	    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Software part of the InspectorWidget framework")
    ELSE()
	    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${DESCRIPTION}")   
    ENDIF()
    IF(NOT CPACK_PACKAGE_DESCRIPTION AND NOT MC_PACKAGE_DESCRIPTION)
	    set(CPACK_PACKAGE_DESCRIPTION "InspectorWidget is a framework for the navigation in multimedia content databases by similarity, using a content-based approach fueled by several feature extraction and clustering algorithms. For more information check http://www.numediart.org")
    ELSE()
	    set(CPACK_PACKAGE_DESCRIPTION "${MC_PACKAGE_DESCRIPTION}")   
    ENDIF()
	SET(CPACK_PACKAGE_VENDOR "InspectorWidget")
	IF(NOT CPACK_PACKAGE_CONTACT)
        set(CPACK_PACKAGE_CONTACT "http://InspectorWidget.be/")
    ENDIF()
	IF(NOT CPACK_PACKAGE_VERSION)
        set(CPACK_PACKAGE_VERSION "${InspectorWidget_VERSION}")
    ENDIF()
	set(CPACK_SOURCE_IGNORE_FILES
		"^${PROJECT_SOURCE_DIR}/Builds/"
	)
    #Unused so far, since we make single application packages instead of a single framework distribution
	#set(CPACK_PACKAGE_EXECUTABLES "InspectorWidgetStudio" "InspectorWidget.icns") #should contain pairs of <executable> and <icon name>
	IF (APPLE)
		#set(CPACK_BUNDLE_NAME "MC")
		set(CPACK_GENERATOR "DragNDrop")#to test: set(CPACK_GENERATOR "PackageMaker;OSXX11")
	ELSE()
		set(CPACK_GENERATOR "TBZ2")
	ENDIF()
ENDIF ( UNIX OR APPLE )

# Borrowed from Performous performous-packaging.cmake CMakeModule
IF(UNIX)
	# Try to find architecture
	execute_process(COMMAND uname -m OUTPUT_VARIABLE CPACK_PACKAGE_ARCHITECTURE)
	string(STRIP "${CPACK_PACKAGE_ARCHITECTURE}" CPACK_PACKAGE_ARCHITECTURE)
	# Try to find distro name and distro-specific arch
	execute_process(COMMAND lsb_release -is OUTPUT_VARIABLE LSB_ID)
	execute_process(COMMAND lsb_release -rs OUTPUT_VARIABLE LSB_RELEASE)
	string(STRIP "${LSB_ID}" LSB_ID)
	string(STRIP "${LSB_RELEASE}" LSB_RELEASE)
	set(LSB_DISTRIB "${LSB_ID}${LSB_RELEASE}")
	IF(NOT LSB_DISTRIB)
		set(LSB_DISTRIB "unix")
	ENDIF(NOT LSB_DISTRIB)

	IF(NOT APPLE)
		SET(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/CPackConfig-${PROGNAME}.cmake")
		SET(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};InspectorWidget;${PROGNAME};/")
		#EXECUTE_PROCESS(COMMAND rm "${CMAKE_BINARY_DIR}/CPackConfig.cmake")
	ENDIF()

	# For Debian-based distros we want to create DEB packages.
	IF("${LSB_DISTRIB}" MATCHES "Ubuntu|Debian")
	    SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "numediart.org")
		set(CPACK_GENERATOR "DEB")
		set(CPACK_DEBIAN_PACKAGE_PRIORITY "extra")
		set(CPACK_DEBIAN_PACKAGE_SECTION "universe/multimedia")
		#set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "..., ...")
		SET(CPACK_DEBIAN_PACKAGE_VERSION "${InspectorWidget_VERSION}")
		SET(CPACK_DEBIAN_PACKAGE_NAME "${PROGNAME}")

		# We need to alter the architecture names as per distro rules
		IF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "i[3-6]86")
			set(CPACK_PACKAGE_ARCHITECTURE i386)
		ENDIF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "i[3-6]86")
		IF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "x86_64")
			set(CPACK_PACKAGE_ARCHITECTURE amd64)
		ENDIF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "x86_64")

		# Set the dependencies based on the distro version

		# Ubuntu
        IF("${LSB_DISTRIB}" MATCHES "Ubuntu")
            set(CPACK_DEBIAN_PACKAGE_MAINTAINER "numediart")
        ENDIF()

	IF("${LSB_DISTRIB}" MATCHES "Ubuntu1") # for our interest, 10.04, 10.10, 11.04

            # Qt4 libqtcore4 and libqtgui4 are package for kubuntu, libqt4-core and libqt4-gui for ubuntu
            IF(USE_QT4 AND WITH_QT)
                list(APPEND UBUNTU_DEPS "libqt4-core|libqtcore4" "libqt4-gui|libqtgui4" "libqt4-opengl" "libqt4-svg" "libqt4-xml")
                IF(QT_USE_PHONON)
                    list(APPEND UBUNTU_DEPS "libphonon4")
                ENDIF()
		IF(QWT_FOUND)
                    list(APPEND UBUNTU_DEPS "libqwt5-qt4|libqwt6")
		ENDIF()
            ENDIF()
 
                # OpenCV
		IF(OpenCV_VERSION VERSION_GREATER 2.3.0)
                    IF(("${LSB_DISTRIB}" MATCHES "Ubuntu10.04") OR ("${LSB_DISTRIB}" MATCHES "Ubuntu11.04") OR ("${LSB_DISTRIB}" MATCHES "Ubuntu11.10"))
                    	#MESSAGE("\n\nWARNING!\nRepackage an up-to-date OpenCV 2.3.1 package from https://launchpad.net/~gijzelaar/+archive/opencv2.3 against recent FFmpeg > 0.8.x packages from https://launchpad.net/~jon-severinsson/+archive/ffmpeg") # no we need to repackage them against more uptodate ffmpeg packages rebuilt from Jon Severinsson's ppa source packages
                    	list(APPEND UBUNTU_DEPS "libopencv-dev")
                    ELSE()
                    	MESSAGE(FATAL_ERROR "OpenCV >= 2.3.0 not available as package for your distribution")
                    ENDIF()
		ELSE()	
                    IF("${LSB_DISTRIB}" MATCHES "Ubuntu10.04")
                        list(APPEND UBUNTU_DEPS "libcv4" "libcvaux4" "libhighgui4")
                    ELSE()
                        list(APPEND UBUNTU_DEPS "libcv2.1" "libcvaux2.1" "libhighgui2.1")
                    ENDIF()
                ENDIF()

            # clucene
            IF(CLUCENE_FOUND)
                list(APPEND UBUNTU_DEPS "libclucene0ldbl")
            ENDIF()

            # PoDoFo
            IF(PODOFO_FOUND)
                IF(("${LSB_DISTRIB}" MATCHES "Ubuntu10.10") OR ("${LSB_DISTRIB}" MATCHES "Ubuntu11.04"))
                    list(APPEND UBUNTU_DEPS "libpodofo0.8.0")
                ELSEIF(("${LSB_DISTRIB}" MATCHES "Ubuntu10.04") OR ("${LSB_DISTRIB}" MATCHES "Ubuntu11.10") OR ("${LSB_DISTRIB}" MATCHES "Ubuntu12.04"))
                    list(APPEND UBUNTU_DEPS "libpodofo0.9.0")
                ENDIF()
            ENDIF()

            STRING(REGEX REPLACE ";" ", " UBUNTU_DEPS "${UBUNTU_DEPS}")
            #MESSAGE("Ubuntu 10.10 deps: ${UBUNTU_DEPS}")
			set(CPACK_DEBIAN_PACKAGE_DEPENDS ${UBUNTU_DEPS})
		ENDIF()

		IF(NOT CPACK_DEBIAN_PACKAGE_DEPENDS)
			message("WARNING: ${LSB_DISTRIB} not supported yet.\nPlease set deps in cmake/CreatePackage.cmake before packaging.")
		ENDIF(NOT CPACK_DEBIAN_PACKAGE_DEPENDS)
		string(TOLOWER "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}-${LSB_DISTRIB}_${CPACK_PACKAGE_ARCHITECTURE}" CPACK_PACKAGE_FILE_NAME)

		# Install the icon file
		INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/icons/InspectorWidget.png DESTINATION share/pixmaps COMPONENT ${PROGNAME} RENAME ${PROGNAME}.png)
		#INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/icons/InspectorWidget.xpm DESTINATION share/pixmaps COMPONENT ${PROGNAME} RENAME ${PROGNAME}.xpm)

		# Install the .desktop description
		file(WRITE ${CMAKE_BINARY_DIR}/${PROGNAME}.desktop [Desktop\ Entry]\nType=Application\nExec=${PROGNAME}\nMimeType=application/x-${PROGNAME};\nIcon=${PROGNAME}\nName=${PROGNAME}\nGenericName=${DESCRIPTION}\nComment=${DESCRIPTION})
		INSTALL(FILES ${CMAKE_BINARY_DIR}/${PROGNAME}.desktop DESTINATION share/applications COMPONENT ${PROGNAME})

	ENDIF("${LSB_DISTRIB}" MATCHES "Ubuntu|Debian")
	# For Fedora-based distros we want to create RPM packages.
	# IF("${LSB_DISTRIB}" MATCHES "Fedora")
	# 	set(CPACK_GENERATOR "RPM")
	# 	set(CPACK_RPM_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
	# 	set(CPACK_RPM_PACKAGE_VERSION "${PROJECT_VERSION}")
	# 	set(CPACK_RPM_PACKAGE_RELEASE "1")
	# 	set(CPACK_RPM_PACKAGE_GROUP "Amusements/Games")
	# 	set(CPACK_RPM_PACKAGE_LICENSE "LGPL?")
	# 	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "InspectorWidget, a framework for navigation by similarity in multimedia databases.")
	# 	set(CPACK_RPM_PACKAGE_DESCRIPTION "InspectorWidget...")
		# We need to alter the architecture names as per distro rules
	# 	IF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "i[3-6]86")
	# 		set(CPACK_PACKAGE_ARCHITECTURE i386)
	# 	ENDIF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "i[3-6]86")
	# 	IF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "x86_64")
	# 		set(CPACK_PACKAGE_ARCHITECTURE amd64)
	# 	ENDIF("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "x86_64")
	# 	# Set the dependencies based on the distro version
	# 	IF("${LSB_DISTRIB}" MATCHES "Fedora14")
	# 		set(CPACK_RPM_PACKAGE_REQUIRES "..., ...")
	# 	ENDIF("${LSB_DISTRIB}" MATCHES "Fedora14")
	# 	IF("${LSB_DISTRIB}" MATCHES "Fedora13")
	# 		set(CPACK_RPM_PACKAGE_REQUIRES "..., ...")
	# 	ENDIF("${LSB_DISTRIB}" MATCHES "Fedora13")
	# 	IF(NOT CPACK_RPM_PACKAGE_REQUIRES)
	# 		message("WARNING: ${LSB_DISTRIB} is not supported.\nPlease set deps in cmake/performous-packaging.cmake before packaging.")
	# 	ENDIF(NOT CPACK_RPM_PACKAGE_REQUIRES)
	#  ENDIF("${LSB_DISTRIB}" MATCHES "Fedora")
	set(CPACK_SYSTEM_NAME "${LSB_DISTRIB}-${CPACK_PACKAGE_ARCHITECTURE}")
ENDIF(UNIX)

#From: http://www.cmake.org/Wiki/BundleUtilitiesExample
SET(plugin_dest_dir bin)
SET(qtconf_dest_dir bin)
SET(qtframeworks_dest_dir bin)
SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${PROGNAME}")
IF(APPLE)
  SET(plugin_dest_dir ${PROGNAME}.app/Contents/PlugIns)
  SET(qtconf_dest_dir ${PROGNAME}.app/Contents/Resources)
  SET(qtframeworks_dest_dir ${PROGNAME}.app/Contents/Resources)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/${PROGNAME}.app")
ENDIF(APPLE)
IF(WIN32)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${PROGNAME}.exe")
ENDIF(WIN32)

#--------------------------------------------------------------------------------
# Install the QtTest application, on Apple, the bundle is at the root of the
# install tree, and on other platforms it'll go into the bin directory.
INSTALL(TARGETS ${PROGNAME}
   BUNDLE DESTINATION . COMPONENT ${PROGNAME}
   RUNTIME DESTINATION bin COMPONENT ${PROGNAME}
)

#--------------------------------------------------------------------------------
# Install needed Qt plugins by copying directories from the qt installation
# One can cull what gets copied by using 'REGEX "..." EXCLUDE'
IF(APPLE)
IF(WITH_QT)
	IF(USE_QT4)
		INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir} COMPONENT ${PROGNAME})
		file(GLOB_RECURSE QTPLUGINS ${QT_PLUGINS_DIR}/imageformats/*.dylib)
		STRING(REGEX REPLACE "${QT_PLUGINS_DIR}" "${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}" QTPLUGINS "${QTPLUGINS}")
	ELSEIF(USE_QT5)
		foreach(plugin ${Qt5Gui_PLUGINS})
		                get_target_property(_loc ${plugin} LOCATION)
		                #message("Core Plugin ${plugin} is at location ${_loc}")
				get_filename_component(plugin_path "${_loc}" PATH)
				get_filename_component(plugin_path "${plugin_path}" NAME)
				#MESSAGE("plugin_path ${plugin_path}")
				INSTALL(FILES "${_loc}" DESTINATION ${plugin_dest_dir}/${plugin_path} COMPONENT ${PROGNAME})				
				get_filename_component(plugin_name "${_loc}" NAME )
				LIST(APPEND QTPLUGINS "${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/${plugin_path}/${plugin_name}")
		endforeach()

		# We need to install Cocoa platform plugin for Qt5 on Mac.
		# FIXME: This should be part of Qt5 CMake scripts, but unfortunatelly
		# Qt5 Mac support is missing there.
		  macro(install_qt5_plugin _qt_plugin_name _qt_plugins_var)
		    get_target_property(_qt_plugin_path "${_qt_plugin_name}" LOCATION)
		    if(EXISTS "${_qt_plugin_path}")
		      get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
		      get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
		      get_filename_component(_qt_plugin_type "${_qt_plugin_type}" NAME)
		      set(_qt_plugin_dest "${plugin_dest_dir}/${_qt_plugin_type}")
		      install(FILES "${_qt_plugin_path}"
		        DESTINATION "${_qt_plugin_dest}"
			COMPONENT ${PROGNAME})
		      set(${_qt_plugins_var}
		        "${${_qt_plugins_var}};${_qt_plugin_dest}/${_qt_plugin_file}")
		    else()
		      message(FATAL_ERROR "QT plugin ${_qt_plugin_name} not found")
		    endif()
		  endmacro()

		  install_qt5_plugin("Qt5::QCocoaIntegrationPlugin" QT_PLUGINS)

		# Install mediaservice plugins, hacking the general qt5 plugin path from Qt5::QCocoaIntegrationPlugin
		get_target_property(_qt_plugin_path "Qt5::QCocoaIntegrationPlugin" LOCATION)
		if(EXISTS "${_qt_plugin_path}")
			get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
			get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
			get_filename_component(_qt_plugin_type "${_qt_plugin_type}" NAME)
			set(_qt_plugin_dest "${plugin_dest_dir}/${_qt_plugin_type}")
			get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
			get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
			get_filename_component(_qt_plugins_path "${_qt_plugin_type}" PATH)
			file(GLOB_RECURSE plugins ${_qt_plugins_path}/mediaservice/*.dylib)
			foreach(plugin ${plugins})
				get_filename_component(plugin_path "${plugin}" PATH)
				get_filename_component(plugin_path "${plugin_path}" NAME)
				get_filename_component(plugin_name "${plugin}" NAME )
				INSTALL(FILES "${plugin}" DESTINATION ${plugin_dest_dir}/${plugin_path} COMPONENT ${PROGNAME})				
				LIST(APPEND QTPLUGINS "${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/${plugin_path}/${plugin_name}")
			endforeach()
		endif()
	ENDIF()
ENDIF()
ENDIF()
#--------------------------------------------------------------------------------
# install a qt.conf file
# this inserts some cmake code into the install script to write the file
IF(APPLE)
IF(WITH_QT)
	INSTALL(CODE "
 	   file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]\nPlugins = plugins\")
 	   " COMPONENT ${PROGNAME})

	IF(USE_QT4)
		FIND_FILE(QT_MENU_NIB qt_menu.nib PATHS /opt/local PATH_SUFFIXES Library/Frameworks/QtGui.framework/Resources lib/Resources)
		IF(${QT_MENU_NIB} EQUAL QT_MENU_NIB-NOTFOUND)
			MESSAGE(FATAL_ERROR "Couldn't find qt_menu.nib")
		ENDIF()
		INSTALL(DIRECTORY "${QT_MENU_NIB}" DESTINATION ${qtframeworks_dest_dir} COMPONENT ${PROGNAME})
	ENDIF()
ENDIF()
ENDIF()

#--------------------------------------------------------------------------------
# Use BundleUtilities to get all other dependencies for the application to work.
# It takes a bundle or executable along with possible plugins and inspects it
# for dependencies.  If they are not system dependencies, they are copied.

# Now the work of copying dependencies into the bundle/package
# The quotes are escaped and variables to use at install time have their $ escaped
# An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
# Note that the image plugins depend on QtSvg and QtXml, and it got those copied
# over.
IF(APPLE)
LIST(APPEND PLUGINS ${EXTRA_APPS})
IF(WITH_QT)
	LIST(APPEND PLUGINS ${QTPLUGINS})
ENDIF()
#MESSAGE(STATUS "${PROGNAME} Qt Plugins: ${QTPLUGINS}")
FOREACH(PLUGIN ${PLUGINS})
	MESSAGE(STATUS "Bundled with ${PROGNAME}: ${PLUGIN}")
ENDFOREACH(PLUGIN)
INSTALL(CODE "
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"${PLUGINS}\" \"${LINKED_DIRECTORIES}\")
    " COMPONENT ${PROGNAME})
ENDIF()

#IF(APPLE)
#	INSTALL(CODE "hdiutil create -format UDBZ -srcfolder \"${CMAKE_INSTALL_PREFIX}/${PROGNAME}.app\" \"${CMAKE_INSTALL_PREFIX}/${PROGNAME}.dmg\"")
#ENDIF()

IF(APPLE)
	# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
	# where CPackConfig.cmake is created by including CPack
	# And then there's ways to customize this as well
	set(CPACK_PACKAGE_NAME "${PROGNAME}")
	set(CPACK_BUNDLE_NAME "${PROGNAME}")
	set(CPACK_BINARY_DRAGNDROP ON)
	set(CPACK_PACKAGE_EXECUTABLES "${PROGNAME}" "InspectorWidget.icns") #should contain pairs of <executable> and <icon name>
	set(CPACK_GENERATOR "PackageMaker;OSXX11")
	#include(CPack)
	#EXECUTE_PROCESS(COMMAND cp "${CMAKE_BINARY_DIR}/CPackConfig.cmake" "${CMAKE_BINARY_DIR}/CPackConfig${PROGNAME}.cmake")
ENDIF()

IF(NOT APPLE)
	include(CPack)
	#EXECUTE_PROCESS(COMMAND rm "${CMAKE_BINARY_DIR}/CPackSourceConfig.cmake")
ENDIF()

ENDIF(NOT USE_DEBUG) # mandatory for packaging release versions
