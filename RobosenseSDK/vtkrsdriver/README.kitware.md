# rs_driver fork for LidarView

This branch contains changes required to embed robosense driver into LidarView
through a VTK module. This includes changes made primarily to the build system
to allow it to be embedded into another source tree.

  * Integrate CMake code with VTK's module system.
  * Add getters for driver params
