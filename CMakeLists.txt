cmake_minimum_required(VERSION 3.5)


set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 13)
set(PROJECT_VERSION_PATCH 2)

project(actiCHamp
	LANGUAGES CXX
	VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
#if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
SET(CMAKE_INSTALL_PREFIX ".\\bin" CACHE PATH "..." FORCE)
#endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "CMAKE_BUILD_TYPE was default initialized to Release")
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited configs" FORCE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(Threads REQUIRED)
find_package(lsl REQUIRED)



if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(platform 64)
	set(actiCHampLib ${CMAKE_CURRENT_LIST_DIR}/actiCHampLib/x64/Release/actiCHampLib.lib)
	set(actiCHampLibDll ${CMAKE_CURRENT_LIST_DIR}/actiCHampLib/x64/Release/actiCHampLib.dll)
	set(ActiChampDll ${CMAKE_CURRENT_LIST_DIR}/actiCHampLib/x64/Release/ActiChamp_x64.dll)
	set(ActiChampXBit ${CMAKE_CURRENT_LIST_DIR}/actiCHampLib/x64/Release/ActiChampX.bit)
	set(ActiChampBit ${CMAKE_CURRENT_LIST_DIR}/actiCHampLib/x64/Release/ActiChamp.bit)
else()
	set(platform 32)
	set(actiCHampLib ${CMAKE_CURRENT_LIST_DIR}/actiCHampLib/Win32/Release/actiCHampLib.lib)
	set(actiCHampLibDll ${CMAKE_CURRENT_LIST_DIR}/actiCHampLib/Win32/Release/actiCHampLib.dll)
	set(ActiChampDll ${CMAKE_CURRENT_LIST_DIR}/actiCHampLib/Win32/Release/ActiChamp_x86.dll)
	set(ActiChampXBit ${CMAKE_CURRENT_LIST_DIR}/actiCHampLib/Win32/Release/ActiChampX.bit)
	set(ActiChampBit ${CMAKE_CURRENT_LIST_DIR}/actiCHampLib/Win32/Release/ActiChamp.bit)

endif()


macro(findQtInstallationTool qtdeploytoolname)
	if(QT_QMAKE_EXE AND EXISTS ${QT_QMAKE_EXE})
		message(STATUS "Qt deploy tool found at ${QT_QMAKE_EXE}")
		return()
	endif()
	get_target_property(QT_QMAKE_EXE Qt5::qmake IMPORTED_LOCATION)
	execute_process(COMMAND ${QT_QMAKE_EXE} -query QT_VERSION OUTPUT_VARIABLE QT_VERSION)
	get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXE}" DIRECTORY)
	find_program (QT_DEPLOYQT_EXECUTABLE ${qtdeploytoolname} HINTS "${QT_BIN_DIR}")
	if (NOT QT_DEPLOYQT_EXECUTABLE)
		message(WARNING "Windeployqt wasn't found, installing ${PROJECT_NAME} will fail!")
		return()
	endif()
endmacro()

add_executable(${PROJECT_NAME} WIN32
	LibTalker.cpp
	LibTalker.h
	main.cpp
	mainwindow.cpp
	mainwindow.h
	mainwindow.ui
	mainwindow.rc
	actiCHampLib/Amplifier_LIB.h 
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
	Qt5::Widgets
	Threads::Threads
	LSL::lsl
	${actiCHampLib}
)	

function(addQtDeployToInstall target)
	set(appbin "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/${target}${CMAKE_EXECUTABLE_SUFFIX}")
	findQtInstallationTool("windeployqt")
	if (QT_DEPLOYQT_EXECUTABLE)
		file (TO_NATIVE_PATH "${QT_BIN_DIR}" QT_BIN_DIR_NATIVE)
		# It's safer to use `\` separators in the Path, but we need to escape them
		string (REPLACE "\\" "\\\\" QT_BIN_DIR_NATIVE "${QT_BIN_DIR_NATIVE}")
		set(QT_DEPLOYQT_FLAGS --no-translations --no-system-d3d-compiler --no-opengl-sw --no-angle --no-svg --no-compiler-runtime)
		file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${target}_$<CONFIG>_path"
			CONTENT "$<TARGET_FILE:${target}>"  # Full path to .exe file
		)
		get_filename_component(appdir ${appbin} DIRECTORY CACHE)
		install (CODE "
			file(READ \"${CMAKE_CURRENT_BINARY_DIR}/${target}_${CMAKE_BUILD_TYPE}_path\" _file)
			message (STATUS \"Running Qt Deploy Tool for \${_file}\")
			if (CMAKE_INSTALL_CONFIG_NAME STREQUAL \"Debug\")
				set(QT_DEPLOYQT_FLAGS \"\${QT_DEPLOYQT_FLAGS} --debug\")
			else ()
				set(QT_DEPLOYQT_FLAGS \"\${QT_DEPLOYQT_FLAGS} --release\")
			endif ()
			execute_process(COMMAND
				\"${CMAKE_COMMAND}\" -E env
				\"Path=${QT_BIN_DIR_NATIVE};\$ENV{SystemRoot}\\\\System32;\$ENV{SystemRoot}\"
				\"${QT_DEPLOYQT_EXECUTABLE}\"
				${QT_DEPLOYQT_FLAGS} --dry-run --list mapping
				\"${appbin}\"
				OUTPUT_VARIABLE output
				OUTPUT_STRIP_TRAILING_WHITESPACE
			)
			string(REPLACE \"\\\\\" \"/\" _output \${output})
			separate_arguments(_files WINDOWS_COMMAND \${_output})
			while(_files)
				list(GET _files 0 _src)
				list(GET _files 1 _dest)
				execute_process(
					COMMAND \"${CMAKE_COMMAND}\" -E
						copy \${_src} \"\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/$\{_dest}\"
				)
				list(REMOVE_AT _files 0 1)
			endwhile()
			" COMPONENT ${target})
			#add_custom_command(TARGET ${target} POST_BUILD
			#	COMMAND "${CMAKE_COMMAND}" -E env PATH="${QT_BIN_DIR}" "${QT_DEPLOYQT_EXECUTABLE}"
			#	${QT_DEPLOYQT_FLAGS}
			#	\"$<TARGET_FILE:${target}>\")
	endif()
	set_property(GLOBAL APPEND PROPERTY
		LSLMENU "${PROJECT_NAME}/${target}.exe" "${target}")
	message(STATUS "MENU appended: ${target}")
endfunction()

install(TARGETS ${PROJECT_NAME}
	COMPONENT "${PROJECT_NAME}"
	BUNDLE DESTINATION ${PROJECT_NAME}
	RUNTIME DESTINATION ${PROJECT_NAME}
	LIBRARY DESTINATION ${PROJECT_NAME}/lib
)
	
install(FILES $<TARGET_FILE:LSL::lsl>
	DESTINATION ${PROJECT_NAME}
	COMPONENT "${PROJECT_NAME}"
)

install(FILES ${PROJECT_NAME}.cfg
	DESTINATION ${PROJECT_NAME}
	COMPONENT "${PROJECT_NAME}"
)

install(FILES ${actiCHampLibDll}
	DESTINATION ${PROJECT_NAME}
	COMPONENT "${PROJECT_NAME}"
)

install(FILES ${ActiChampDll}
	DESTINATION ${PROJECT_NAME}
	COMPONENT "${PROJECT_NAME}"
)

install(FILES ${ActiChampXBit}
	DESTINATION ${PROJECT_NAME}
	COMPONENT "${PROJECT_NAME}"
)

install(FILES ${ActiChampBit}
	DESTINATION ${PROJECT_NAME}
	COMPONENT "${PROJECT_NAME}"
)

addQtDeployToInstall(${PROJECT_NAME})

if (WIN32)
	get_target_property(QT5_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
	get_filename_component(QT5_WINDEPLOYQT_EXECUTABLE ${QT5_QMAKE_EXECUTABLE} PATH)
	set(QT5_WINDEPLOYQT_EXECUTABLE "${QT5_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")

	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${QT5_WINDEPLOYQT_EXECUTABLE} --qmldir
			${CMAKE_SOURCE_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different  
			$<TARGET_FILE:LSL::lsl>
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_SOURCE_DIR}//${PROJECT_NAME}.cfg
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			${actiCHampLibDll}
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			${ActiChampDll}
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			${ActiChampXBit}
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			${ActiChampBit}
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_SOURCE_DIR}/release_notes.txt
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_SOURCE_DIR}/LICENSE.txt
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_SOURCE_DIR}/explanation_of_trigger_marker_types.pdf
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif(WIN32)

# CPack configuration
include(CPackComponent)
cpack_add_component(${PROJECT_NAME}
	DISPLAY_NAME ${PROJECT_NAME}
	DESCRIPTION "${PROJECT_NAME} ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

macro(LSLGenerateCPackConfig)
	# top level CMakeLists.txt?
	if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
		# CPack configuration
		set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
		set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
		set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
		set(CPACK_STRIP_FILES ON)
		set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
		set(CPACK_PACKAGE_NAME lsl)

		set(CPACK_GENERATOR "7Z") # you can create NSIS packages by calling 'cpack -G NSIS'
		set(CPACK_NSIS_MODIFY_PATH ON)
		set(LSL_OS "Win${platform}")
		get_property(CPACK_NSIS_MENU_LINKS GLOBAL PROPERTY LSLMENU)
		message(STATUS "MENU: ${CPACK_NSIS_MENU_LINKS}")


		get_cmake_property(CPACK_COMPONENTS_ALL COMPONENTS)
		foreach(component ${CPACK_COMPONENTS_ALL})
			string(TOUPPER ${component} COMPONENT)
			message(STATUS "Setting packages name for ${COMPONENT}")
			set(LSL_CPACK_FILENAME "${component}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-${LSL_OS}")
			message(STATUS "File name: ${LSL_CPACK_FILENAME}")
			set("CPACK_DEBIAN_${COMPONENT}_FILE_NAME" "${LSL_CPACK_FILENAME}.deb")
			set("CPACK_ARCHIVE_${COMPONENT}_FILE_NAME" ${LSL_CPACK_FILENAME})
			#set(CPACK_DEBIAN_${component}_FILE_NAME "${FILENAME}.deb")
		endforeach()

		message(STATUS "Installing Components: ${CPACK_COMPONENTS_ALL}")
		include(CPack)
	endif()
endmacro()

LSLGenerateCPackConfig()