paraview_add_plugin(RobosensePlugin
  REQUIRED_ON_CLIENT
  REQUIRED_ON_SERVER
  VERSION "1.0"
  MODULES
    RobosensePlugin::RobosensePacketInterpreters
    RobosensePlugin::RobosenseSDK
  MODULE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/RobosensePacketInterpreters/vtk.module"
    "${CMAKE_CURRENT_SOURCE_DIR}/RobosenseSDK/vtk.module"
  SERVER_MANAGER_XML
    "${CMAKE_CURRENT_SOURCE_DIR}/RobosenseProxies.xml"
)

