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

/// \file GPUTPCStartHitsSorter.h
/// \author David Rohr

#ifndef GPUTPCSTARTHITSSORTER_H
#define GPUTPCSTARTHITSSORTER_H

#include "GPUTPCDef.h"
#include "GPUTPCHitId.h"
#include "GPUGeneralKernels.h"
#include "GPUConstantMem.h"

namespace GPUCA_NAMESPACE
{
namespace gpu
{
MEM_CLASS_PRE()
class GPUTPCTracker;

/**
 * @class GPUTPCStartHitsSorter
 *
 */
class GPUTPCStartHitsSorter
{
 public:
  MEM_CLASS_PRE()
  class GPUTPCSharedMemory
  {
    friend class GPUTPCStartHitsSorter;

   public:
#if !defined(GPUCA_GPUCODE)
    GPUTPCSharedMemory() : mStartRow(0), mNRows(0), mStartOffset(0)
    {
    }

    GPUTPCSharedMemory(const GPUTPCSharedMemory& /*dummy*/) : mStartRow(0), mNRows(0), mStartOffset(0) {}
    GPUTPCSharedMemory& operator=(const GPUTPCSharedMemory& /*dummy*/) { return *this; }
#endif //! GPUCA_GPUCODE

   protected:
    int mStartRow;    // start row index
    int mNRows;       // number of rows to process
    int mStartOffset; // start offset for hits sorted by this block
  };

  typedef GPUconstantref() MEM_GLOBAL(GPUTPCTracker) processorType;
  GPUhdi() static GPUDataTypes::RecoStep GetRecoStep() { return GPUCA_RECO_STEP::TPCSliceTracking; }
  MEM_TEMPLATE()
  GPUhdi() static processorType* Processor(MEM_TYPE(GPUConstantMem) & processors)
  {
    return processors.tpcTrackers;
  }
  template <int iKernel = 0>
  GPUd() static void Thread(int nBlocks, int nThreads, int iBlock, int iThread, GPUsharedref() MEM_LOCAL(GPUTPCSharedMemory) & smem, processorType& tracker);
};
} // namespace gpu
} // namespace GPUCA_NAMESPACE

#endif // GPUTPCSTARTHITSSORTER_H
