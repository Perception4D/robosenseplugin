/*=========================================================================

  Program: LidarView
  Module:  vtkRobosenseLidarReader.h

  Copyright 2018 (c) Kitware Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef vtkRobosenseLidarReader_h
#define vtkRobosenseLidarReader_h

#include <vtkLidarReader.h>

#include "RobosenseIOModule.h"

/**
 * @class vtkRobosenseLidarReader
 *
 * Reads pcap files using vtkLidarPacketInterpreter implementations.
 */
class ROBOSENSEIO_EXPORT vtkRobosenseLidarReader : public vtkLidarReader
{
public:
  static vtkRobosenseLidarReader* New();
  vtkTypeMacro(vtkRobosenseLidarReader, vtkLidarReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /**
   * Set/Get calibration port.
   */
  vtkGetMacro(CalibrationPort, int);
  void SetCalibrationPort(int port);
  ///@}

protected:
  vtkRobosenseLidarReader();
  ~vtkRobosenseLidarReader() override;

  ///@{
  /**
   * Open/Close the pcap file.
   */
  virtual bool Open(bool reassemble = true) override;
  ///@}

private:
  vtkRobosenseLidarReader(const vtkRobosenseLidarReader&) = delete;
  void operator=(const vtkRobosenseLidarReader&) = delete;

  int CalibrationPort = -1;
};

#endif
