/*=========================================================================

  Program: LidarView
  Module:  vtkRobosenseLidarStream.h

  Copyright 2018 (c) Kitware Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef vtkRobosenseLidarStream_h
#define vtkRobosenseLidarStream_h

#include "vtkLidarStream.h"

#include "RobosenseIOModule.h"

/**
 * @class vtkRobosenseLidarStream
 *
 * Reads pcap files using vtkLidarPacketInterpreter implementations.
 */
class ROBOSENSEIO_EXPORT vtkRobosenseLidarStream : public vtkLidarStream
{
public:
  static vtkRobosenseLidarStream* New();
  vtkTypeMacro(vtkRobosenseLidarStream, vtkLidarStream)

  /**
   * Start the stream with the lidar & calibration port
   */
  void Start() override;

  ///@{
  /**
   * Set/Get calibration port.
   */
  vtkGetMacro(CalibrationPort, unsigned int);
  vtkSetMacro(CalibrationPort, unsigned int);
  ///@}

protected:
  vtkRobosenseLidarStream();
  ~vtkRobosenseLidarStream();

private:
  vtkRobosenseLidarStream(const vtkRobosenseLidarStream&) = delete;
  void operator=(const vtkRobosenseLidarStream&) = delete;

  /*!< The port to receive information*/
  unsigned int CalibrationPort = 8308;
};

#endif // vtkRobosenseLidarStream_h
