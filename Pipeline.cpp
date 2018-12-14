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

	isTrue = true;
}

//TODO handle superscalar...
void Pipeline::retire(){
	for(int i = 0; i < mWidth; i++) {
		if (mROB[mHead].rdy) {
			mHead = (mHead + 1) % mROBsize;
		}
	}
}


void Pipeline::writeback(){
	for(int i = 0; i < mWidth*5; i++)
	{
		//Only handle new writeback requests
		if (mWBPR[i].valid){
			//Tell the ROB this is ready to leave
			mROB[mWBPR[i].robID].rdy = true;
			//Set this request as read
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
		//If processor is not in use skip to next one
		if(!mFU[i].inUse) {break;}
		//Reduce the necessary cycles left to run
		mFU[i].cyclesLeft--;
		//if the instruction has finished executing
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
				if( mFU[i].fuInstr.robID == mIQ[j].iqInstr.rs1 && mIQ[j].iqInstr.rs1ROB)
				{
					mIQ[j].rs1Rdy = &isTrue;
				}

				if( mFU[i].fuInstr.robID == mIQ[j].iqInstr.rs2 && mIQ[j].iqInstr.rs2ROB)
				{
					mIQ[j].rs2Rdy = &isTrue;
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
			if (mIQ[j].valid && *(mIQ[j].rs1Rdy) && *(mIQ[j].rs2Rdy)){
				//Update the oldest tracking
				if (mIQ[j].age > maxAge){
					oldest = j;
					maxAge = mIQ[j].age;
				}
				//Update the ages of instructions
				mIQ[j].age = mIQ[j].age + 1;
			}
		}

		//Take the oldest ready instruction out of the issue queue
		mIQ[oldest].valid = false;
		mIQentries--;

		//Find an open processing unit for the oldest instr
		bool fuFound = false;
		while(!fuFound) {
			//on finding a ready processor
			if(!mFU[k].inUse){
				//Set that we found a processor
				fuFound = true;
				//Give it the instruction
				mFU[k].fuInstr = mIQ[oldest].iqInstr;
				//Set it to used
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
			//If not available move to next unit
			k++;
		}
	}
}

void Pipeline::dispatch() {
	int i = 0;
	//There are available slots in the issue queue
	if(mIQentries+mWidth < mIQsize) {
		mDEStall = false;
		for( int i = 0; i < mWidth; i++) {
			for (int j = 0; j < mIQsize; j++) {
				if (!mIQ[j].valid) {
					//Insert instruction into instruction queue
					mIQ[j].iqInstr = mDIPR[i];
					mIQ[j].valid = true;
					mIQ[j].age = 1;
					mIQentries++;

					//Init readiness of values
					//If the value is in ARF it is ready
					if (!mDIPR[i].rs1ROB)
					{
						mIQ[j].rs1Rdy = &isTrue;
					}
					else
					{
						//If it is in the rob attach the pointers
						mIQ[j].rs1Rdy = &(mROB[mDIPR[i].rs1].rdy);
					}
					//Same for RS2
					if (!mDIPR[i].rs2ROB)
					{
						mIQ[j].rs2Rdy = &isTrue;
					}
					else
					{
						//mIQ[0].rs1Rdy = &(mROB[0].rdy);
						//If it is in the rob attach the pointers
						mIQ[j].rs2Rdy = &(mROB[mDIPR[i].rs2].rdy);
					}

					if(mDIPR[i].rs2)
					break;
				}
			}
		}
	}
	//If there are no available slots stall the processors early stages
	else
	{
		mDEStall= true;
	}
}

void Pipeline::regRead() {
	if(mDEStall) {
		return;
	}

	//Pass along the bundle;
	for(int i = 0; i < mWidth; i++) {
		mDIPR[i] = mRRPR[i];
	}
}

void Pipeline::rename() {
	//If IQ is full stall
	if(mDEStall) {
		return;
	}

	//If ROB is full stall
	if (mHead <= (mTail + mWidth) % mROBsize) {
		mRNStall = true;
		return;
	}

	mRNStall = false;

	//Allocate spot in ROB
	for(int i = 0; i < mWidth; i++) {
		mTail++;
		mRNPR[i].robID = mTail;
		mROB[mTail].dst = mRNPR[i].rd;
		mROB[mTail].pc = mRNPR[i].pc;
		mROB[mTail].rdy = false;

		//Rename src registers
		if(mRMT[mRNPR[i].rs1].valid)
		{
			//Get the ROB tag from the RMT
			mRNPR[i].rs1 = mRMT[mRNPR[i].rs1].tag;
			//This value keeps track of whether or not the register has been renamed
			mRNPR[i].rs1ROB = true;
		}
		else
		{
			mRNPR[i].rs1ROB = false;
		}

		if(mRMT[mRNPR[i].rs2].valid)
		{
			mRNPR[i].rs2 = mRMT[mRNPR[i].rs2].tag;
			mRNPR[i].rs2ROB = true;
		}
		else
		{
			mRNPR[i].rs2ROB = false;
		}

		//Update RMT for dest register
		mRMT[mRNPR[i].rd].tag = mRNPR[i].robID;
	}

	//Pass along the bundle
	for(int i = 0; i < mWidth; i++) {
		mRRPR[i] = mRNPR[i];
	}
}

void Pipeline::decode() {
	//Do nothing if stalled
	if (mDEStall || mRNStall)
	{
		return;
	}

	//Otherwise just pass the bundle along
	for(int i = 0; i < mWidth; i++) {
		mRNPR[i] = mDEPR[i];
	}
}

//TODO handle bundles that are not full...
void Pipeline::fetch(instr* input) {
	//Do nothing if stalled
	if (mDEStall || mRNStall)
	{
		return;
	}

	//Pass the bundle along
	for(int i = 0; i < mWidth; i++) {
		mDEPR[i] = input[i];
	}
}
