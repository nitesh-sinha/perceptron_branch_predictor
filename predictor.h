#ifndef _PREDICTOR_H_
#define _PREDICTOR_H_

#include "utils.h"
#include "tracer.h"


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class PREDICTOR{

  // The state is defined for Gshare, change for your design

 private:
  UINT64  ghr;           // global history register
  UINT32  *pht;          // pattern history table
  UINT32  numPhtEntries; // entries in pht 
  
  // Added by Nitesh for perceptron predictor
  UINT32 numPerceptronEntries;
  INT32 **perceptronTable;
  INT32 trainingThreshold;
  INT32 sum; // sum output of perceptron

 public:

  // The interface to the four functions below CAN NOT be changed

  PREDICTOR(void);
  bool    GetPrediction(UINT32 PC);  
  void    UpdatePredictor(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget);
  void    TrackOtherInst(UINT32 PC, OpType opType, UINT32 branchTarget);

  // Contestants can define their own functions below
};



/***********************************************************/
#endif

