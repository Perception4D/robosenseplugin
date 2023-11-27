/*=========================================================================

  Program:   LidarView
  Module:    vtkRobosensePacketInterpreter.h

  Copyright (c) Kitware Inc.
  All rights reserved.
  See LICENSE or http://www.apache.org/licenses/LICENSE-2.0 for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef vtkRobosensePacketInterpreter_h
#define vtkRobosensePacketInterpreter_h

#include <vtkLidarPacketInterpreter.h>

#include <vtkStringArray.h>
#include <vtkTypeInt64Array.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkUnsignedShortArray.h>

#include <memory>

#include "RobosensePacketInterpretersModule.h"

class ROBOSENSEPACKETINTERPRETERS_EXPORT vtkRobosensePacketInterpreter
  : public vtkLidarPacketInterpreter
{
public:
  static vtkRobosensePacketInterpreter* New();
  vtkTypeMacro(vtkRobosensePacketInterpreter, vtkLidarPacketInterpreter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void Initialize() override;

  void ProcessPacket(unsigned char const* data, unsigned int dataLength) override;

  bool IsLidarPacket(unsigned char const* data, unsigned int dataLength) override;

  bool PreProcessPacket(unsigned char const* data,
    unsigned int dataLength,
    double &outLidarDataTime) override;

  void SetLidarModel(int type);
  vtkGetMacro(LidarModel, int);

protected:
  vtkSmartPointer<vtkPolyData> CreateNewEmptyFrame(vtkIdType nbrOfPoints,
    vtkIdType prereservedNbrOfPoints = 60000) override;

  vtkSmartPointer<vtkPoints> Points;
  vtkSmartPointer<vtkDoubleArray> PointsX;
  vtkSmartPointer<vtkDoubleArray> PointsY;
  vtkSmartPointer<vtkDoubleArray> PointsZ;
  vtkSmartPointer<vtkUnsignedCharArray> Intensity;
  vtkSmartPointer<vtkUnsignedCharArray> LaserId;
  vtkSmartPointer<vtkDoubleArray> Distance;
  vtkSmartPointer<vtkTypeInt64Array> Timestamp;

  int LidarModel = 0;
  unsigned int LastTimestamp = 0;

  vtkRobosensePacketInterpreter();
  ~vtkRobosensePacketInterpreter() = default;

private:
  vtkRobosensePacketInterpreter(const vtkRobosensePacketInterpreter&) = delete;
  void operator=(const vtkRobosensePacketInterpreter&) = delete;

  bool SplitFrame(uint16_t height, double timestamp);

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;
};

#endif // vtkRobosensePacketInterpreter_h
