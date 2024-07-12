/*=========================================================================

  Program:   LidarView
  Module:    vtkRobosensePacketInterpreter.cxx

  Copyright (c) Kitware Inc.
  All rights reserved.
  See LICENSE or http://www.apache.org/licenses/LICENSE-2.0 for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkRobosensePacketInterpreter.h"
#include "InterpreterHelper.h"

#include <vtk_rs_driver.h>

#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>

#include <cstring>

namespace rs_lidar = robosense::lidar;

typedef PointXYZIRT PointT;
typedef PointCloudT<PointT> RBPointCloud;

namespace
{
constexpr unsigned int RS_PACKET_SIZE = 1248;
}

//-----------------------------------------------------------------------------
class vtkRobosensePacketInterpreter::vtkInternals
{
public:
  vtkInternals()
    : RobosensePointCloud(std::make_shared<RBPointCloud>())
  {
  }

  bool IsDifopPacket(const uint8_t* pkt)
  {
    const rs_lidar::RSDecoderConstParam& param = this->GetDecoderConstParam();
    return std::memcmp(pkt, param.DIFOP_ID, param.DIFOP_ID_LEN) == 0;
  }

  bool IsMsopPacket(const uint8_t* pkt)
  {
    const rs_lidar::RSDecoderConstParam& param = this->GetDecoderConstParam();
    return std::memcmp(pkt, param.MSOP_ID, param.MSOP_ID_LEN) == 0;
  }

  void CreateDecoder(rs_lidar::LidarType& lidarType, const rs_lidar::RSDecoderParam& decoderParams)
  {
    // TODO: Check if is valid type? != 0
    this->LidarType = lidarType;
    this->Decoder = rs_lidar::DecoderFactory<RBPointCloud>::createDecoder(lidarType, decoderParams);
    this->Decoder->point_cloud_ = this->RobosensePointCloud;
    this->Decoder->getDecoderConstParam(this->DecoderConstParam);
  }

  const rs_lidar::LidarType& GetLidarType() const { return this->LidarType; }

  std::shared_ptr<rs_lidar::Decoder<RBPointCloud>>& GetDecoder() { return this->Decoder; }

  const rs_lidar::RSDecoderConstParam& GetDecoderConstParam() const
  {
    return this->DecoderConstParam;
  }

  std::shared_ptr<RBPointCloud>& GetCloud() { return this->RobosensePointCloud; }

  void RunExceptionCallback(const robosense::lidar::Error& error) const
  {
    vtkGenericWarningMacro("Robosense driver internal error: " << error.toString());
  }

private:
  rs_lidar::LidarType LidarType = rs_lidar::RS16;
  std::shared_ptr<rs_lidar::Decoder<RBPointCloud>> Decoder;
  rs_lidar::RSDecoderConstParam DecoderConstParam;
  std::shared_ptr<RBPointCloud> RobosensePointCloud;
};

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkRobosensePacketInterpreter)

//-----------------------------------------------------------------------------
vtkRobosensePacketInterpreter::vtkRobosensePacketInterpreter()
  : Internals(new vtkRobosensePacketInterpreter::vtkInternals())
{
  this->ResetCurrentFrame();
  this->SetSensorVendor("Robosense");
  this->SetSensorModelName(rs_lidar::lidarTypeToStr(this->Internals->GetLidarType()));
}

//-----------------------------------------------------------------------------
void vtkRobosensePacketInterpreter::ProcessPacket(unsigned char const* data,
  unsigned int dataLength)
{
  if (dataLength != ::RS_PACKET_SIZE)
  {
    return;
  }

  auto& internals = this->Internals;
  const uint8_t* pkt = reinterpret_cast<const uint8_t*>(data);

  if (internals->IsMsopPacket(pkt))
  {
    internals->GetDecoder()->processMsopPkt(pkt, dataLength);
  }
  else if (internals->IsDifopPacket(pkt))
  {
    internals->GetDecoder()->processDifopPkt(pkt, dataLength);
  }
  else
  {
    vtkWarningMacro("Not a Msop nor a Difop packet.");
  }
}

//-----------------------------------------------------------------------------
bool vtkRobosensePacketInterpreter::IsLidarPacket(unsigned char const* data,
  unsigned int dataLength)
{
  if (dataLength != ::RS_PACKET_SIZE)
  {
    return false;
  }

  // Check for DIFOP packet and save adapter
  const uint8_t* pkt = reinterpret_cast<const uint8_t*>(data);
  if (this->Internals->IsDifopPacket(pkt))
  {
    // auto& adapter = this->Internals->AdapterDifop;
    // rs_lidar::RS16DifopPkt2Adapter(rs16DifopPkt, adapter);
    // this->Rpm = ntohs(adapter.rpm);
    this->Internals->GetDecoder()->processDifopPkt(pkt, dataLength);
    Superclass::Initialize();
    // If this is true Difop packets will be called by ProcessPacket
    return true;
  }

  return this->Internals->IsMsopPacket(pkt);
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> vtkRobosensePacketInterpreter::CreateNewEmptyFrame(
  vtkIdType nbrOfPoints,
  vtkIdType prereservedNbrOfPoints)
{
  const int defaultPrereservedNbrOfPointsPerFrame = 60000;
  // prereserve for 50% points more than actually received in previous frame
  prereservedNbrOfPoints =
    std::max(static_cast<int>(prereservedNbrOfPoints * 1.5), defaultPrereservedNbrOfPointsPerFrame);

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();

  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  points->Allocate(prereservedNbrOfPoints);
  if (nbrOfPoints > 0)
  {
    points->SetNumberOfPoints(nbrOfPoints);
  }
  points->GetData()->SetName("Points");
  polyData->SetPoints(points.GetPointer());

  this->Points = points.GetPointer();
  // clang-format off
  InitArrayForPolyData(true, this->PointsX, "X", nbrOfPoints, prereservedNbrOfPoints, polyData, this->EnableAdvancedArrays);
  InitArrayForPolyData(true, this->PointsY, "Y", nbrOfPoints, prereservedNbrOfPoints, polyData, this->EnableAdvancedArrays);
  InitArrayForPolyData(true, this->PointsZ, "Z", nbrOfPoints, prereservedNbrOfPoints, polyData, this->EnableAdvancedArrays);
  InitArrayForPolyData(false, this->Intensity, "intensity", nbrOfPoints, prereservedNbrOfPoints, polyData);
  InitArrayForPolyData(false, this->LaserId, "laser_id", nbrOfPoints, prereservedNbrOfPoints, polyData);
  InitArrayForPolyData(false, this->Timestamp, "timestamp", nbrOfPoints, prereservedNbrOfPoints, polyData);
  InitArrayForPolyData(false, this->Distance, "distance_m", nbrOfPoints, prereservedNbrOfPoints, polyData);
  // clang-format on

  // Set the default array to display in the application
  polyData->GetPointData()->SetActiveScalars("intensity");
  return polyData;
}

//-----------------------------------------------------------------------------
bool vtkRobosensePacketInterpreter::PreProcessPacket(unsigned char const* data,
  unsigned int dataLength,
  double& outLidarDataTime)
{
  if (dataLength != ::RS_PACKET_SIZE)
  {
    return false;
  }

  auto& internals = this->Internals;

  const uint8_t* pkt = reinterpret_cast<const uint8_t*>(data);

  bool splitPacket = false;
  if (internals->IsMsopPacket(pkt) && this->GetIsInitialized())
  {
    splitPacket = internals->GetDecoder()->processMsopPkt(pkt, dataLength);
  }

  outLidarDataTime = internals->GetDecoder()->prevPktTs();

  // In pre-processing we do not want to keep points.
  // Ideally we should do all process in one step, reading the pcap
  // twice slow the pcap reader.
  internals->GetCloud()->points.clear();
  return splitPacket;
}

//-----------------------------------------------------------------------------
void vtkRobosensePacketInterpreter::SetLidarModel(int model)
{
  if (this->LidarModel == model)
  {
    return;
  }

  rs_lidar::LidarType type = static_cast<rs_lidar::LidarType>(model);
  if (rs_lidar::isMech(type) || rs_lidar::isMems(type))
  {
    this->LidarModel = model;
    this->SetSensorModelName(rs_lidar::lidarTypeToStr(type));
    this->Modified();
        this->ResetInitializedState();

  }
}

//-----------------------------------------------------------------------------
void vtkRobosensePacketInterpreter::Initialize()
{
  auto& internals = this->Internals;
  // Robosense sensors are not forced to use calibration files. (But they could)
  rs_lidar::LidarType type = static_cast<rs_lidar::LidarType>(this->LidarModel);
  // Default parameters
  rs_lidar::RSDecoderParam decoderParams;
  internals->CreateDecoder(type, decoderParams);

  // clang-format off
  internals->GetDecoder()->regCallback(
    std::bind(&vtkRobosensePacketInterpreter::vtkInternals::RunExceptionCallback, internals.get(), std::placeholders::_1),
    std::bind(&vtkRobosensePacketInterpreter::SplitFrame, this, std::placeholders::_1, std::placeholders::_2));
  // clang-format on
}

//-----------------------------------------------------------------------------
bool vtkRobosensePacketInterpreter::SplitFrame(uint16_t vtkNotUsed(height),
  double vtkNotUsed(timestamp))
{
  auto& internals = this->Internals;
  const rs_lidar::RSDecoderConstParam& param = internals->GetDecoderConstParam();
  // Number of point in a set of blocks
  const uint32_t numberOfPktPoints = param.BLOCKS_PER_PKT * param.CHANNELS_PER_BLOCK;

  auto& cloud = internals->GetCloud();

  // Prevent from splitting at the start of a frame.
  if (cloud->points.size() <= numberOfPktPoints)
  {
    return false;
  }

  for (auto& point : cloud->points)
  {
    double pos[3] = { point.x, point.y, point.z };
    if (std::isnan(point.x) || std::isnan(point.y) || std::isnan(point.z))
    {
      continue;
    }
    double distance =
      std::sqrt(std::pow(0 - point.x, 2) + std::pow(0 - point.y, 2) + std::pow(0 - point.z, 2));
    this->Points->InsertNextPoint(pos);
    InsertNextValueIfNotNull(this->PointsX, pos[0]);
    InsertNextValueIfNotNull(this->PointsY, pos[1]);
    InsertNextValueIfNotNull(this->PointsZ, pos[2]);
    InsertNextValueIfNotNull(this->Intensity, point.intensity);
    InsertNextValueIfNotNull(this->LaserId, point.ring);
    InsertNextValueIfNotNull(this->Timestamp, static_cast<double>(point.timestamp) / 1e-3);
    InsertNextValueIfNotNull(this->Distance, distance);
  }
  cloud->points.clear();
  return this->Superclass::SplitFrame();
}

//----------------------------------------------------------------------------
void vtkRobosensePacketInterpreter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
