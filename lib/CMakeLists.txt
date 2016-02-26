set(TARGET_NAME "InspectorWidgetProcessorLibrary")
find_package(OpenCL)
if(OPENCL_FOUND)
	add_definitions(-DHAVE_OPENCL)
endif()
if(OpenCV_FOUND AND Tesseract_FOUND)
	file(GLOB SRC *.cpp *.c)
	file(GLOB HDR *.hpp *.h)

	set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

	add_library(${TARGET_NAME} ${SRC} ${HDR})
	target_link_libraries(${TARGET_NAME} ${OpenCV_LIBRARIES} ${Tesseract_LIBRARY} PEGTL)

	set_target_properties("${TARGET_NAME}" PROPERTIES FOLDER "${FOLDERNAME}")
	message("[X] ${TARGET_NAME}")
else()
	message("[ ] ${TARGET_NAME}")
endif()
