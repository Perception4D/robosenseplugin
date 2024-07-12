/*=========================================================================

  Program: LidarView
  Module:  vtkRobosenseLidarReader.cxx

  Copyright (c) Kitware Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkRobosenseLidarReader.h"

#include <vtkObjectFactory.h>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkRobosenseLidarReader)

//-----------------------------------------------------------------------------
vtkRobosenseLidarReader::vtkRobosenseLidarReader() = default;

//-----------------------------------------------------------------------------
vtkRobosenseLidarReader::~vtkRobosenseLidarReader() = default;

//-----------------------------------------------------------------------------
bool vtkRobosenseLidarReader::Open(bool reassemble)
{
  std::vector<int> ports;
  if (this->GetLidarPort() != -1)
  {
    ports.emplace_back(this->GetLidarPort());
    ports.emplace_back(this->CalibrationPort);
  }
  return Superclass::Open(ports, reassemble);
}

//-----------------------------------------------------------------------------
void vtkRobosenseLidarReader::SetCalibrationPort(int port)
{
  if (this->CalibrationPort != port)
  {
    this->CalibrationPort = port;
    this->Modified();
    this->ResetFrameIndexes();
  }
}

//------------------------------------------------------------------------------
void vtkRobosenseLidarReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
