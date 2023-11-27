set(rs_plugin_classes
  vtkRobosensePacketInterpreter
)

vtk_module_add_module(RobosensePlugin::RobosensePacketInterpreters
  CLASSES ${rs_plugin_classes}
)

paraview_add_server_manager_xmls(
  MODULE RobosensePlugin::RobosensePacketInterpreters
  XMLS
    RobosensePacketInterpreter.xml
)
