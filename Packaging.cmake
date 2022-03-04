set(CPACK_IFW_ROOT $ENV{HOME}/Qt/QtIFW-3.0.6/ CACHE PATH "Qt Installer Framework installation base path")
find_program(BINARYCREATOR_EXECUTABLE binarycreator HINTS "${_qt_bin_dir}" ${CPACK_IFW_ROOT}/bin)

set(CPACK_PACKAGE_VENDOR "Brain Products")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_CONTACT "Example_vendor <bci.consultant@brainproducts.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}")

# set human names to exetuables
set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME}" "actiCHamp LSL Connector")
set(CPACK_CREATE_DESKTOP_LINKS "${PROJECT_NAME}")
set(CPACK_STRIP_FILES TRUE)

#------------------------------------------------------------------------------
# include CPack, so we get target for packages
set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/BundleConfig.cmake")

add_custom_target(bundle
                  COMMAND ${CMAKE_CPACK_COMMAND} "--config" "${CMAKE_BINARY_DIR}/BundleConfig.cmake"
                  COMMENT "Running CPACK. Please wait..."
                  DEPENDS ./bin)
set(CPACK_GENERATOR)

# Qt IFW packaging framework
if(BINARYCREATOR_EXECUTABLE)
    list(APPEND CPACK_GENERATOR IFW)
    message(STATUS "   + Qt Installer Framework               YES ")
else()
    message(STATUS "   + Qt Installer Framework                NO ")
endif()


#--------------------------------------------------------------------------
# Windows specific
# needed for wix handling of custom install dir
set(INSTALL_DIR ${CMAKE_PACKAGE_INSTALL_DIRECTORY})
list(APPEND CPACK_GENERATOR ZIP)
message(STATUS "Package generation - Windows")
message(STATUS "   + ZIP                                  YES ")

set(PACKAGE_ICON "${CMAKE_SOURCE_DIR}/Resources/BV_LSL.ico")
set(INSTALLER_HEADER "${CMAKE_SOURCE_DIR}/Resources/Logo.bmp")
set(PREFERRED_INSTALL_DIR "C:\\\\Vision\\\\LSL-Apps\\\\")
# NSIS windows installer
find_program(NSIS_PATH nsis PATH_SUFFIXES nsis)
if(NSIS_PATH)
	list(APPEND CPACK_GENERATOR NSIS)
	message(STATUS "   + NSIS                                 YES ")

	set(CPACK_NSIS_DISPLAY_NAME ${CPACK_PACKAGE_NAME})
	# Icon of the installer
	file(TO_CMAKE_PATH "${PACKAGE_ICON}" CPACK_NSIS_MUI_ICON)
	file(TO_CMAKE_PATH "${PACKAGE_ICON}" CPACK_NSIS_MUI_UNIICON)
	file(TO_CMAKE_PATH "${INSTALLER_HEADER}" CPACK_NSIS_MUI_HEADERIMAGE_BITMAP)
	file(TO_CMAKE_PATH "${INSTALLER_HEADER}" CPACK_NSIS_MUI_WELCOMEFINISH_BITMAP)
	message(STATUS dir:  " ${PREFERRED_INSTALL_DIR}${INSTALL_DIR}\\BrainAmpSeries.exe")
	set(CPACK_NSIS_INSTALLED_ICON_NAME "\\\\bin\\\\BrainAmpSeries.exe")
	set(CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}")
	set(CPACK_NSIS_INSTALL_ROOT "${PREFERRED_INSTALL_DIR}" )
	set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
	set(CPACK_NSIS_MODIFY_PATH ON)
	set(CPACK_CREATE_DESKTOP_LINKS "${PROJECT_NAME}.exe")
	set(CPACK_NSIS_BRANDING_TEXT "Copyright Brain Products 2021")
else()
	message(STATUS "   + NSIS                                 NO ")
endif()

# WIX windows installer
# find_program(WIX_PATH wix PATH_SUFFIXES wix)
# if(WIX_PATH)
	#list(APPEND CPACK_GENERATOR WIX)
	#message(STATUS "   + WIX                                 YES ")
	#file(TO_CMAKE_PATH "${PACKAGE_ICON}" CPACK_WIX_PRODUCT_ICON)
	#set(CMAKE_PACKAGE_INSTALL_DIRECTORY #"${PREFERRED_INSTALL_DIR}${CMAKE_PACKAGE_INSTALL_DIRECTORY}")
	#set(CPACK_WIX_SKIP_PROGRAM_FOLDER TRUE)
	
# else()
	# message(STATUS "   + WIX                                 NO ")
# endif()
# reset the package install dir for nuget
set(CMAKE_PACKAGE_INSTALL_DIRECTORY "${INSTALL_DIR}")

# NuGet package
# find_program(NUGET_EXECUTABLE nuget)
set(NUGET_EXECUTABLE OFF)
if(NUGET_EXECUTABLE)
	list(APPEND CPACK_GENERATOR NuGET)
	message(STATUS "   + NuGET                               YES ")
	set(CPACK_NUGET_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_NUGET_PACKAGE_VERSION "1.0.0")
set(CPACK_NUGET_PACKAGE_DESCRIPTION "Example")
set(CPACK_NUGET_PACKAGE_AUTHORS "Example")
else()
	message(STATUS "   + NuGET                                NO ")
endif()

windeployqt(${PROJECT_NAME})



include(CPack)