
#########################################
################ Library ################
#########################################
file(GLOB SOURCES
	${SRC_ROOT}/main.cpp
	${SRC_ROOT}/helper/*_test.cpp
)

#file(GLOB TEST_FILES ${SRC_ROOT}/../test/*_test.cpp)
set(TEST_FILES "")
if (ENABLE_TEST)
	# Extract test file, by find _test in filename.
	foreach (SOURCE_FILE ${SOURCES})
		string(FIND ${SOURCE_FILE} "_test.cpp" FIND_RESULT)
		if (NOT ${FIND_RESULT} STREQUAL "-1")
			list(APPEND TEST_FILES ${SOURCE_FILE})
			list(REMOVE_ITEM SOURCES ${SOURCE_FILE})
		endif ()
	endforeach ()
endif ()

message(STATUS "Sources: ${SOURCES}")
message(STATUS "TEST_FILES: ${TEST_FILES}")



##############################################
################ Build Target ################
##############################################
add_executable(main main.cpp)
target_link_libraries(main PRIVATE benchmark::benchmark pthread Boost::boost)



############################################
################ Build Test ################
############################################
if (ENABLE_TEST)
	list(LENGTH TEST_FILES TEST_FILE_NUM)
	#	message(STATUS "TEST_FILE_NUM: ${TEST_FILE_NUM}")
	if (NOT ${TEST_FILE_NUM} STREQUAL "0")
		include(GoogleTest)
	endif ()
	foreach (TEST_FILE ${TEST_FILES})
		# [check again]?
		get_filename_component(target ${TEST_FILE} NAME_WLE)
		add_executable(${target} ${TEST_FILE})

		target_link_libraries(${target}
			PRIVATE
			${PRIVATE_MODULE_LIBRARIES_LIST}

			PUBLIC
			${PUBLIC_MODULE_LIBRARIES_LIST}
			pthread rt z
			)

		target_link_libraries(${target}
			PRIVATE
			GTest::gtest_main)
		gtest_discover_tests(${target})
	endforeach ()
endif ()





# add_executable(bug bug.cpp)
# target_link_libraries(bug PRIVATE pthread)