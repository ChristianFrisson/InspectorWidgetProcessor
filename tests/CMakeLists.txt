#List the files and if there are directories, add them to the list of (potential) test programs
message("\nTests:")
set(FOLDERNAME "tests")
file(GLOB DIRLIST * )
foreach(TESTDIR ${DIRLIST})
	if(IS_DIRECTORY ${TESTDIR})
		add_subdirectory(${TESTDIR})
	endif()
endforeach()
