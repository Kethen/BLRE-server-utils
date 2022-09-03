# adjust configuration types to either
#   - Server: Debug/x86; Starts BLR server in debugger
#   - Client: Debug/x86; Starts BLR client in debugger
#   - Release: Release/x86; Compile for release
    set(CMAKE_SHARED_LINKER_FLAGS_SERVER ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
    set(CMAKE_SHARED_LINKER_FLAGS_CLIENT ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
    set(CMAKE_CONFIGURATION_TYPES Server Client Release)
    set(CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} CACHE STRING "Append user-defined configuration to list of configurations to make it usable in Visual Studio" FORCE)

# install dependencies with cpm
include(cmake/CPM.cmake)
include(cmake/packages.cmake)

# configure debugger
function(configure_blrv_env TARG)
    # copy build to BLR directory
    set(OUTPUT "${BLR_BIN_DIR}\\Modules\\${TARG}.dll")
    add_custom_command(TARGET ${TARG} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARG}> ${OUTPUT})

    # add debugger configuration
    set_target_properties(${TARG} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${BLR_BIN_DIR}"
        VS_DEBUGGER_COMMAND "${BLR_BIN_DIR}\\${BLR_EXECUTABLE}"
        VS_DEBUGGER_COMMAND_ARGUMENTS $<$<CONFIG:Server>:${BLR_SERVER_CLI}>$<$<CONFIG:Client>:${BLR_CLIENT_CLI}>
    )

    # add preprocessor definitions
    target_compile_definitions(${TARG} PUBLIC 
        $<$<CONFIG:DEBUG>:DEBUG>
        PROJECT_NAME="${PROJECT_NAME}"
    )
endfunction()

# set cached variables
if(NOT DEFINED BLR_DIRECTORY)
    set(BLR_DIRECTORY "C:\\Program Files (x86)\\Steam\\steamapps\\common\\blacklightretribution" CACHE STRING "Path to BLR installation")
endif()
if(NOT DEFINED BLR_BIN_DIR)
    set(BLR_BIN_DIR "${BLR_DIRECTORY}\\Binaries\\Win32" CACHE STRING "Path to BLR binary directory")
endif()
if(NOT DEFINED BLR_EXECUTABLE)
    set(BLR_EXECUTABLE "BLR.exe" CACHE STRING "BLR filename")
endif()
if(NOT DEFINED BLR_SERVER_CLI)
    set(BLR_SERVER_CLI "server HeloDeck" CACHE STRING "Server URL")
endif()
if(NOT DEFINED BLR_CLIENT_CLI)
    set(BLR_CLIENT_CLI "127.0.0.1?Name=superewald" CACHE STRING "Client URL")
endif()
if(NOT DEFINED SERVER_MODULE)
    set(SERVER_MODULE true CACHE STRING "wether module runs on server side")
endif()
if(NOT DEFINED CLIENT_MODULE)
    set(CLIENT_MODULE true CACHE STRING "wether module runs on client side")
endif()

# include user config override if exists
if(EXISTS "${PROJECT_SOURCE_DIR}/blrevive.config.cmake")
    include("${PROJECT_SOURCE_DIR}/blrevive.config.cmake")
endif()

# create header and source file lists
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")
list(APPEND sources "${PROJECT_SOURCE_DIR}/source/${PROJECT_NAME}.rc")
configure_file("${PROJECT_SOURCE_DIR}/.gitlab/ci/module-info.tpl.json" "${PROJECT_SOURCE_DIR}/source/module.json")
