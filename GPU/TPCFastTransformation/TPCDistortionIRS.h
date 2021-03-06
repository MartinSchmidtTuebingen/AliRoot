//**************************************************************************\
//* This file is property of and copyright by the ALICE Project            *\
//* ALICE Experiment at CERN, All rights reserved.                         *\
//*                                                                        *\
//* Primary Authors: Matthias Richter <Matthias.Richter@ift.uib.no>        *\
//*                  for The ALICE HLT Project.                            *\
//*                                                                        *\
//* Permission to use, copy, modify and distribute this software and its   *\
//* documentation strictly for non-commercial purposes is hereby granted   *\
//* without fee, provided that the above copyright notice appears in all   *\
//* copies and that both the copyright notice and this permission notice   *\
//* appear in the supporting documentation. The authors make no claims     *\
//* about the suitability of this software for any purpose. It is          *\
//* provided "as is" without express or implied warranty.                  *\
//**************************************************************************

/// \file  TPCDistortionIRS.h
/// \brief Definition of TPCDistortionIRS class
///
/// \author  Sergey Gorbunov <sergey.gorbunov@cern.ch>

#ifndef ALICEO2_GPUCOMMON_TPCFASTTRANSFORMATION_TPCDISTORTIONIRS_H
#define ALICEO2_GPUCOMMON_TPCFASTTRANSFORMATION_TPCDISTORTIONIRS_H

#include "IrregularSpline2D3D.h"
#include "FlatObject.h"
#include "GPUCommonDef.h"

namespace GPUCA_NAMESPACE
{
namespace gpu
{

///
/// The TPCDistortionIRS class represents correction of nominal coordinates of TPC clusters
/// using irregular splines
///
/// Row, U, V -> dX,dU,dV
///
/// The class is flat C structure. No virtual methods, no ROOT types are used.
///
class TPCDistortionIRS : public FlatObject
{
 public:
  ///
  /// \brief The struct contains necessary info for TPC padrow
  ///
  struct RowInfo {
    float x;                ///< x coordinate of the row [cm]
    float U0;               ///< min. u coordinate
    float scaleUtoSU;       ///< scale for su coordinate
    float scaleSUtoU;       ///< scale for u coordinate
    int splineScenarioID;   ///< scenario index (which of IrregularSpline2D3D splines to use)
    size_t dataOffsetBytes; ///< offset for the spline data withing a TPC slice
  };

  /// _____________  Constructors / destructors __________________________

  /// Default constructor: creates an empty uninitialized object
  TPCDistortionIRS();

  /// Copy constructor: disabled to avoid ambiguity. Use cloneTo[In/Ex]ternalBuffer() instead
  TPCDistortionIRS(const TPCDistortionIRS&) CON_DELETE;

  /// Assignment operator: disabled to avoid ambiguity. Use cloneTo[In/Ex]ternalBuffer() instead
  TPCDistortionIRS& operator=(const TPCDistortionIRS&) CON_DELETE;

  /// Destructor
  ~TPCDistortionIRS() CON_DEFAULT;

  /// _____________  FlatObject functionality, see FlatObject class for description  ____________

  /// Memory alignment

  using FlatObject::getBufferAlignmentBytes;
  using FlatObject::getClassAlignmentBytes;

  /// Construction interface

  void cloneFromObject(const TPCDistortionIRS& obj, char* newFlatBufferPtr);
  void destroy();

  /// Making the data buffer external

  using FlatObject::releaseInternalBuffer;
  void moveBufferTo(char* newBufferPtr);

  /// Moving the class with its external buffer to another location

  void setActualBufferAddress(char* actualFlatBufferPtr);
  void setFutureBufferAddress(char* futureFlatBufferPtr);

  /// _______________  Construction interface  ________________________

  /// Starts the construction procedure, reserves temporary memory
  void startConstruction(int numberOfRows, int numberOfScenarios);

  /// Initializes a TPC row
  void setTPCrow(int iRow, float x, int nPads, float padWidth, int iScenario);

  /// Sets TPC geometry
  void setTPCgeometry(float tpcLengthSideA, float tpcLengthSideC);

  /// Sets approximation scenario
  void setApproximationScenario(int scenarioIndex, const IrregularSpline2D3D& spline);

  /// Finishes construction: puts everything to the flat buffer, releases temporary memory
  void finishConstruction();

  /// _______________  Initialization interface  ________________________

  /// Sets the time stamp of the current calibaration
  void setTimeStamp(long int v) { mTimeStamp = v; }

  /// Gives pointer to a spline
  GPUd() const IrregularSpline2D3D& getSpline(int slice, int row) const;

  /// Gives pointer to spline data
  GPUd() float* getSplineDataNonConst(int slice, int row);

  /// Gives pointer to spline data
  GPUd() const float* getSplineData(int slice, int row) const;

  /// Gives minimal alignment in bytes required for the class object
  static constexpr size_t getClassAlignmentBytes() { return 8; }

  /// Gives minimal alignment in bytes required for the flat buffer
  static constexpr size_t getBufferAlignmentBytes() { return 8; }

  /// _______________ The main method: cluster distortion  _______________________
  ///
  GPUd() int getDistortion(int slice, int row, float u, float v, float& dx, float& du, float& dv) const;

  /// _______________  Utilities  _______________________________________________

  /// Gives number of TPC slices
  static int getNumberOfSlices() { return NumberOfSlices; }

  /// Gives number of TPC rows
  int getNumberOfRows() const { return mNumberOfRows; }

  /// Gives the time stamp of the current calibaration parameters
  long int getTimeStamp() const { return mTimeStamp; }

  /// Gives TPC row info
  GPUd() const RowInfo& getRowInfo(int row) const { return mRowInfoPtr[row]; }

  GPUd() int convUVtoSUV(int slice, int row, float u, float v, float& su, float& sv) const;
  GPUd() int convSUVtoUV(int slice, int row, float su, float sv, float& u, float& v) const;

  /// Print method
  void Print() const;

 private:
  void relocateBufferPointers(const char* oldBuffer, char* newBuffer);

  /// Enumeration of construction states
  enum ConstructionExtraState : unsigned int {
    GeometryIsSet = 0x4 ///< geometry is set
  };

  /// _______________  Data members  _______________________________________________

  static constexpr int NumberOfSlices = 36; ///< Number of TPC slices ( slice = inner + outer sector )

  /// _______________  Construction control  _______________________________________________

  int mConstructionCounterRows;                                  ///< counter for constructed members
  int mConstructionCounterScenarios;                             ///< counter for constructed members
  std::unique_ptr<RowInfo[]> mConstructionRowInfos;              ///< Temporary container of the row infos during construction
  std::unique_ptr<IrregularSpline2D3D[]> mConstructionScenarios; ///< Temporary container for spline scenarios

  /// _______________  Geometry  _______________________________________________

  int mNumberOfRows;      ///< Number of TPC rows. It is different for the Run2 and the Run3 setups
  int mNumberOfScenarios; ///< Number of approximation spline scenarios

  RowInfo* mRowInfoPtr;              ///< pointer to RowInfo array inside the mFlatBufferPtr buffer
  IrregularSpline2D3D* mScenarioPtr; ///< Pointer to spline scenarios

  float mScaleVtoSVsideA; ///< scale for v->sv for TPC side A
  float mScaleVtoSVsideC; ///< scale for v->sv for TPC side C
  float mScaleSVtoVsideA; ///< scale for sv->v for TPC side A
  float mScaleSVtoVsideC; ///< scale for sv->v for TPC side C

  /// _______________  Calibration data  _______________________________________________

  long int mTimeStamp; ///< time stamp of the current calibration

  char* mSplineData;          ///< pointer to the spline data in the flat buffer
  size_t mSliceDataSizeBytes; ///< size of the data for one slice in the flat buffer
};

/// ====================================================
///       Inline implementations of some methods
/// ====================================================

GPUdi() int TPCDistortionIRS::convUVtoSUV(int slice, int row, float u, float v, float& su, float& sv) const
{
  const RowInfo& rowInfo = getRowInfo(row);
  su = (u - rowInfo.U0) * rowInfo.scaleUtoSU;
  if (slice < 18) {
    sv = v * mScaleVtoSVsideA;
  } else {
    sv = v * mScaleVtoSVsideC;
  }
  return 0;
}

GPUdi() int TPCDistortionIRS::convSUVtoUV(int slice, int row, float su, float sv, float& u, float& v) const
{
  const RowInfo& rowInfo = getRowInfo(row);
  u = rowInfo.U0 + su * rowInfo.scaleSUtoU;
  if (slice < 18) {
    v = sv * mScaleSVtoVsideA;
  } else {
    v = sv * mScaleSVtoVsideC;
  }
  return 0;
}

GPUdi() int TPCDistortionIRS::getDistortion(int slice, int row, float u, float v, float& dx, float& du, float& dv) const
{
  const IrregularSpline2D3D& spline = getSpline(slice, row);
  const float* splineData = getSplineData(slice, row);
  float su = 0, sv = 0;
  convUVtoSUV(slice, row, u, v, su, sv);
  spline.getSplineVec(splineData, su, sv, dx, du, dv);
  return 0;
}

GPUdi() const IrregularSpline2D3D& TPCDistortionIRS::getSpline(int slice, int row) const
{
  /// Gives pointer to spline
  const RowInfo& rowInfo = mRowInfoPtr[row];
  return mScenarioPtr[rowInfo.splineScenarioID];
}

GPUdi() float* TPCDistortionIRS::getSplineDataNonConst(int slice, int row)
{
  /// Gives pointer to spline data
  const RowInfo& rowInfo = mRowInfoPtr[row];
  return reinterpret_cast<float*>(mSplineData + mSliceDataSizeBytes * slice + rowInfo.dataOffsetBytes);
}

GPUdi() const float* TPCDistortionIRS::getSplineData(int slice, int row) const
{
  /// Gives pointer to spline data
  const RowInfo& rowInfo = mRowInfoPtr[row];
  return reinterpret_cast<float*>(mSplineData + mSliceDataSizeBytes * slice + rowInfo.dataOffsetBytes);
}
} // namespace gpu
} // namespace GPUCA_NAMESPACE

#endif
