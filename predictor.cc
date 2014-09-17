#include "predictor.h"
#include <math.h>

#define PHT_CTR_MAX  3
#define PHT_CTR_INIT 2

// Knobs for perceptron predictor
#define HIST_LEN   63
#define PERCEPTRON_INDEX_BITS 9 // 2^PERCEPTRON_INDEX_BITS = no. of PERCEPTRON_ENTRIES


/////////////// STORAGE BUDGET JUSTIFICATION ////////////////
// Total storage budget: 32KB + 1024 bits
// Perceptron table size: 2 ^ PERCEPTRON_INDEX_BITS = 2^9=512 entries
// Size of all the weigths in the perceptron table: 512 * 64 * 8 bits = 256 Kbits or 32 KB
// GHR size: 63 bits
// Total Size = total perceptron table size + GHR size
/////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

PREDICTOR::PREDICTOR(void){

  ghr              = 0xFFFFFFFF;
  numPerceptronEntries = (1<< PERCEPTRON_INDEX_BITS);
  trainingThreshold = 1.93 * HIST_LEN + 14; // optimum value
  sum=0;
  
  perceptronTable = new INT32*[numPerceptronEntries];
  
  for (UINT32 i=0; i<numPerceptronEntries; i++)
  {
	perceptronTable[i]= new INT32[HIST_LEN+1];  // for the extra weight w0
  }

   for (UINT32 i=0; i<numPerceptronEntries; i++)
   {
     for(int j=0;j<HIST_LEN+1;j++)
      {
	perceptronTable[i][j]= 0;  
      }
   }
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool   PREDICTOR::GetPrediction(UINT32 PC){

  // Get the hash of the PC and use it to index the table of Perceptrons.
  // Select the perceptron and multiply the weights obtained from perceptron 
  // with GHR(with w0=1) to get Y. If Y>=0, return TAKEN else return Not Taken
  
  UINT32 pc_hash=0,temp_PC=0,tempghr;
  INT32 *perceptron_entry;
  
  UINT32 numShift;	//no. of shifts to GHR to get the branch histories
  
  temp_PC=PC>> PERCEPTRON_INDEX_BITS;
  pc_hash=(PC % numPerceptronEntries) ^ (temp_PC % numPerceptronEntries); // XORing LSB bits of PC with higher order bits of PC to get pc_hash
  //pc_hash=(PC % numPerceptronEntries);
  perceptron_entry = perceptronTable[pc_hash];
  sum = 1 * perceptron_entry[0]; // Bias weight = 1 always

  for(UINT32 i=0; i<HIST_LEN; i++){

	  numShift=i;
	tempghr=ghr>>numShift;

    if(tempghr % 2) // if xi=1 => Taken branch, so add it to the sum. Else subtract it from the sum
	   sum+= perceptron_entry[i+1];
	else
	   sum-= perceptron_entry[i+1];
  }

	if(sum>=0)
	return TAKEN;
	else
	return NOT_TAKEN;

  
}


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void  PREDICTOR::UpdatePredictor(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget){

  // If resoveDir != PredDir OR sum <= trainingThreshold, then increment the weights
  // of the perceptron predictor. Write the updated perceptron_entry into the perceptronTable.
  // Also update ghr with the resolveDir
  
  UINT32 pc_hash=0,temp_PC=0,tempghr;
  INT32 *perceptron_entry;
  INT32 t;
  
  UINT32 numShift;
  temp_PC=PC>> PERCEPTRON_INDEX_BITS;
  pc_hash=(PC % numPerceptronEntries) ^ (temp_PC % numPerceptronEntries); // XORing LSB bits of PC with higher order bits of PC to get pc_hash
  //pc_hash=(PC % numPerceptronEntries);
  perceptron_entry = perceptronTable[pc_hash];
  
  
  if(resolveDir)
	t=1;
  else
	t=-1;
  
  if(resolveDir != predDir || abs(sum) <= trainingThreshold)
  {
	// Update the first weight based on x0 which is always 1
	  if(t==1)
	  {
		if(perceptron_entry[0]<127)
		   perceptron_entry[0] = perceptron_entry[0] + t;
	  }
	  else if(t==-1)
	  {
		if(perceptron_entry[0]>-128)
		   perceptron_entry[0] = perceptron_entry[0] + t;
	  }

	
    for(UINT32 i=0; i< HIST_LEN; i++)
	{
	  // Update weights of the remaining perceptron entries
      tempghr=ghr>>numShift;
	  numShift=i;

      if((tempghr % 2) == resolveDir)
      {    // saturate incremement the weight
    	  if(perceptron_entry[i]<127)
    		  perceptron_entry[i]+=1;
      }

	  else
	  {    // saturate decremement the weight
    	  if(perceptron_entry[i]>-128)
    		  perceptron_entry[i]-=1;
	  }

	}
   }
   
   
   // update the GHR
  ghr = (ghr << 1);

  if(resolveDir == TAKEN){
    ghr++; 
  }
  
  sum=0;

}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void    PREDICTOR::TrackOtherInst(UINT32 PC, OpType opType, UINT32 branchTarget){

  // This function is called for instructions which are not
  // conditional branches, just in case someone decides to design
  // a predictor that uses information from such instructions.
  // We expect most contestants to leave this function untouched.

  return;
}
