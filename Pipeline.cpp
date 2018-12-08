/*
 * Pipeline.cpp
 *
 *  Created on: Dec 7, 2018
 *      Author: Barrett Bryson
 */

#include "Pipeline.h"

Pipeline::Pipeline(unsigned long int width, unsigned long int iq_size, unsigned long int rob_size) {
	mWidth = width;
	mARF = new int[67];

	mROBsize = rob_size;
	mIQsize = iq_size;
	mIQentries = 0;

	mROB = new rob_entry[rob_size];
	mIQ = new iq_entry[iq_size];
	mRMT = new rmt_entry[67];

	mDEPR = new instr[width];
    mRNPR = new instr[width];
	mRRPR = new instr[width];
	mDIPR = new instr[width];
	mFU = new fu[width*5];
	mWBPR = new instr[width*5];


	mHead = 0;
	mTail = 0;
}

//TODO handle superscalar...
void Pipeline::retire(){
	if (mROB[mHead].rdy) {
		mHead = (mHead + 1) % mROBsize;
	}
}

//handled with pointers?
void Pipeline::writeback(){
	for(int i = 0; i < mWidth*5; i++)
	{
		if (mWBPR[i].valid){
			mROB[mWBPR[i].robID].rdy = true;
			mWBPR[i].valid = false;
		}
	}
}

//This was being worked on
void Pipeline::execute(){
	//Iterate through all instructions currently in ex stage
	int j = 0;
	for(int i = 0; i < mWidth*5; i++)
	{
		//TODO only do this if it is inuse?
		mFU[i].cyclesLeft--;
		if(mFU[i].cyclesLeft == 0)
		{
			//set the fu to available
			mFU[i].inUse = false;

			//Send finished instruction to writeback
			mWBPR[j] = mFU[i].fuInstr;
			j++;

			//Send wakeups into IQ
			for(int k = 0; k < mIQsize; k++)
			{
				if( mFU[i].fuInstr.robID == mIQ[j].rs1Val && mIQ[j].rs1ROB)
				{
					mIQ[j].rs1Rdy = true;
				}

				if( mFU[i].fuInstr.robID == mIQ[j].rs2Val && mIQ[j].rs2ROB)
				{
					mIQ[j].rs1Rdy = true;
				}
			}
		}
	}

}

void Pipeline::issue() {
	int k = 0;
	for(int i = 0; i < mWidth; i++) {

		int maxAge = 0;
		int oldest = 0;
		//Find the oldest instruction in IQ
		for(int j = 0; j < mIQsize; j++) {
			//Only look for valid and ready instructions
			if (mIQ[j].valid && mIQ[j].rs1Rdy && mIQ[j].rs2Rdy){
				if (mIQ[j].age > maxAge){
					oldest = j;
					maxAge = mIQ[j].age;
					mIQ[j].valid = false;
					mIQentries--;
				}
			}
		}

		//Find an open processing unit for the oldest instr
		bool fuFound = false;
		while(!fuFound) {
			if(!mFU[k].inUse){
				mFU[k].fuInstr = mIQ[oldest].iqInstr;
				mFU[k].inUse = true;
				//Set the number of cycles by optype
				switch(mIQ[oldest].iqInstr.op_type) {
				case 0:
					mFU[k].cyclesLeft = 1;
					break;

				case 1:
					mFU[k].cyclesLeft = 2;
					break;

				case 2:
					mFU[k].cyclesLeft = 5;
					break;
				}
			}
		}
	}
}

void Pipeline::dispatch() {


}

void Pipeline::regRead() {

}

void Pipeline::rename() {

}

void Pipeline::decode() {

}

void Pipeline::fetch() {

}
