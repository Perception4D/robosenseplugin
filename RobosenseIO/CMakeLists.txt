set(rs_plugin_classes
  vtkRobosenseLidarReader
  vtkRobosenseLidarStream
)

vtk_module_add_module(RobosensePlugin::RobosenseIO
  CLASSES ${rs_plugin_classes}
)

paraview_add_server_manager_xmls(
  MODULE RobosensePlugin::RobosenseIO
  XMLS
    RobosenseLidarReader.xml
    RobosenseLidarStream.xml
)
