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
	mROBentries = 0;
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

	mDIStall = false;
	mRNStall = false;

	isTrue = true;

	eof = false;
	finished = false;

	cycle = 0;
}

void Pipeline::retire(){
	//update validation cycle counts for time spent in retire
	for(int j = 0; j < mROBsize; j++) {
		mROB[j].robInstr.rtDur++;
	}

	//Retire width ready instructions from the ROB
	for(int i = 0; i < mWidth; i++) {
		if(mROBentries == 0) { break; }
		if (mROB[mHead].rdy) {

			//Clear RMT if necessary
			if(mRMT[mROB[mHead].dst].tag == mHead && mROB[mHead].dst != -1)
			{
				mRMT[mROB[mHead].dst].valid = false;
			}

			//Update anyone in iq
			for(int k = 0; k < mIQsize; k++)
			{
				if( mHead == mIQ[k].iqInstr.rs1 && mIQ[k].iqInstr.rs1ROB)
				{
					mIQ[k].rs1Rdy = true;
				}

				if( mHead == mIQ[k].iqInstr.rs2 && mIQ[k].iqInstr.rs2ROB)
				{
					mIQ[k].rs2Rdy = true;
				}
			}

			//Cover instructions that have been renamed but have not yet entered issue queue
			for(int k = 0; k < mWidth; k++)
			{
				//Renamed instructions
				if( mHead == mRRPR[k].rs1 && mRRPR[k].rs1ROB)
				{
					mRRPR[k].rs1ROB = false;
				}
				if( mHead == mRRPR[k].rs2 && mRRPR[k].rs2ROB)
				{
					mRRPR[k].rs2ROB = false;
				}

				//Dispatching instructions
				if( mHead == mDIPR[k].rs1 && mDIPR[k].rs1ROB)
				{
					mDIPR[k].rs1ROB = false;
				}
				if( mHead == mDIPR[k].rs2 && mDIPR[k].rs2ROB)
				{
					mDIPR[k].rs2ROB = false;
				}
			}

			//Print out the Validation info
			instr in = mROB[mHead].robInstr;
			printf("%d fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} RR{%d,%d} DI{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d} RT{%d,%d}\n",
					in.traceLine, in.op_type, in.rs1Orig, in.rs2Orig, in.rd, in.feEnt, in.feDur, in.deEnt, in.deDur, in.rnEnt, in.rnDur, in.rrEnt,
					in.rrDur, in.diEnt, in.diDur, in.isEnt, in.isDur, in.exEnt, in.exDur, in.wbEnt, in.wbDur, in.rtEnt, in.rtDur);

			//Move the head pointer
			mHead = (mHead + 1) % mROBsize;
			mROBentries--;
		}
	}
	//If the file has reached the end and the ROB is empty then the process is complete.
	if (eof && mROBentries == 0) {
		finished = true;
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
			//update validation counts
			mWBPR[i].rtEnt = cycle + 1;
			mWBPR[i].wbDur++;
			mROB[mWBPR[i].robID].robInstr = mWBPR[i];

			//Send wakeups into IQ
			for(int k = 0; k < mIQsize; k++)
			{
				if( mWBPR[i].robID == mIQ[k].iqInstr.rs1 && mIQ[k].iqInstr.rs1ROB)
				{
					mIQ[k].rs1Rdy = true;
				}

				if( mWBPR[i].robID == mIQ[k].iqInstr.rs2 && mIQ[k].iqInstr.rs2ROB)
				{
					mIQ[k].rs2Rdy = true;
				}
			}
		}
	}
}

void Pipeline::execute(){
	//Iterate through all instructions currently in ex stage
	int j = 0;
	for(int i = 0; i < mWidth*5; i++)
	{
		//If processor is not in use skip to next one
		if(!mFU[i].inUse) {continue;}
		//Reduce the necessary cycles left to run
		mFU[i].cyclesLeft--;
		//update cycle count for validation
		mFU[i].fuInstr.exDur++;
		//if the instruction has finished executing
		if(mFU[i].cyclesLeft == 0)
		{
			//set writeback entrance time
			mFU[i].fuInstr.wbEnt = cycle + 1;

			//set the fu to available
			mFU[i].inUse = false;

			//Send finished instruction to writeback
			mWBPR[j] = mFU[i].fuInstr;
			j++;
		}

		if(mFU[i].cyclesLeft == 0)
		{
			//Send wakeups into IQ
			for(int k = 0; k < mIQsize; k++)
			{
				if( mFU[i].fuInstr.robID == mIQ[k].iqInstr.rs1 && mIQ[k].iqInstr.rs1ROB)
				{
					mIQ[k].rs1Rdy = true;
				}

				if( mFU[i].fuInstr.robID == mIQ[k].iqInstr.rs2 && mIQ[k].iqInstr.rs2ROB)
				{
					mIQ[k].rs2Rdy = true;
				}
			}
		}
	}

}

void Pipeline::issue() {
	int k = 0;
	//Update cycle times and ages
	for(int j = 0; j < mIQsize; j++) {
		if (mIQ[j].valid)
		{
			mIQ[j].iqInstr.isDur++;
			mIQ[j].age = mIQ[j].age + 1;
		}
	}

	//Attempt to issue the oldest instructions
	for(int i = 0; i < mWidth; i++) {
		//skip if issue queue is empty
		if (mIQentries == 0){
			return;
		}
		int maxAge = 0;
		int oldest = -1;
		//Find the oldest instruction in IQ
		for(int j = 0; j < mIQsize; j++) {
			//Only look for valid and ready instructions
			if (mIQ[j].valid && mIQ[j].rs1Rdy && mIQ[j].rs2Rdy){
				//Update the oldest tracking
				if (mIQ[j].age > maxAge){
					oldest = j;
					maxAge = mIQ[j].age;
				}
			}
		}

		//no instructions are ready for issue
		if (oldest == -1) { return;	}
		//Take the oldest ready instruction out of the issue queue
		mIQ[oldest].valid = false;
		mIQentries--;

		//Find an open processing unit for the oldest instr
		bool fuFound = false;
		while(!fuFound) {
			//on finding a ready processor
			if(!mFU[k].inUse){
				//Set that it found a processor
				fuFound = true;
				//Give it the instruction
				mFU[k].fuInstr = mIQ[oldest].iqInstr;
				mFU[k].fuInstr.exEnt = cycle + 1;
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
	//There are available slots in the issue queue
	if(mIQentries+mWidth <= mIQsize) {
		mDIStall = false;
		for( int i = 0; i < mWidth; i++) {
			//End the stage if there are no valid instructions left in the pipeline register
			if (!mDIPR[i].valid){
				//printf("previous stall invalidated the instructions this cycle\n");
				return;
			}
			//Search for open IQ slot
			for (int j = 0; j < mIQsize; j++) {
				if (!mIQ[j].valid) {
					//Insert instruction into instruction queue
					mIQ[j].iqInstr = mDIPR[i];
					mIQ[j].valid = true;
					mIQ[j].age = 1;
					mIQentries++;

					mIQ[j].iqInstr.isEnt = cycle + 1;
					mIQ[j].iqInstr.diDur++;

					//Init readiness of values
					//If the value is in ARF it is ready
					if (!mDIPR[i].rs1ROB)
					{
						mIQ[j].rs1Rdy = true;
					}
					else
					{
						//If it is in the rob and ready
						if(mROB[mDIPR[i].rs1].rdy)
						{
							mIQ[j].rs1Rdy = true;
						}
						else
						{
							mIQ[j].rs1Rdy = false;
						}
					}
					//Same for RS2
					if (!mDIPR[i].rs2ROB)
					{
						mIQ[j].rs2Rdy = true;
					}
					else
					{
						//If it is in the rob and ready
						if(mROB[mDIPR[i].rs2].rdy)
						{
							mIQ[j].rs2Rdy = true;
						}
						else
						{
							mIQ[j].rs2Rdy = false;
						}
					}

					//End the loop we found a IQ entry for it
					j = mIQsize;
				}
			}
		}
	}
	//If there are no available slots stall the processors early stages
	else
	{
		for(int i = 0; i < mWidth; i++) {
			mDIPR[i].diDur++;
		}
		mDIStall= true;
	}
}

void Pipeline::regRead() {
	//if stalled just update cycle count
	if(mDIStall) {
		for(int i = 0; i < mWidth; i++) {
			mRRPR[i].rrDur++;
		}
		return;
	}

	//Pass along the bundle;
	for(int i = 0; i < mWidth; i++) {
		mDIPR[i] = mRRPR[i];
		mDIPR[i].diEnt = cycle + 1;
		mDIPR[i].rrDur++;
	}
}

void Pipeline::rename() {
	//If IQ is full stall
	if(mDIStall) {
		for(int i = 0; i < mWidth; i++) {
			mRNPR[i].rnDur++;
		}
		return;
	}

	//If ROB is full stall
	if (mROBentries + mWidth > mROBsize) {
		mRNStall = true;
		for(int i = 0; i < mWidth; i++) {
			mRNPR[i].rnDur++;
		}

		for(int i = 0; i < mWidth; i++) {
			mRRPR[i].valid = false;
		}
		return;
	}

	//No longer stalled
	mRNStall = false;

	//Allocate spot in ROB
	for(int i = 0; i < mWidth; i++) {
		//skip if not a valid instruction
		if(!mRNPR[i].valid) {
			//Pass along the bundle
			for(int i = 0; i < mWidth; i++) {
				mRRPR[i] = mRNPR[i];
				mRRPR[i].rrEnt = cycle + 1;
				mRRPR[i].rnDur++;
			}
			return;
		}

		//Place values into the ROB
		mRNPR[i].robID = mTail;
		mROB[mTail].dst = mRNPR[i].rd;
		mROB[mTail].pc = mRNPR[i].pc;
		mROB[mTail].rdy = false;
		mROB[mTail].robInstr = mRNPR[i];

		//increment the ROB tail
		mTail = (mTail + 1) % mROBsize;
		mROBentries++;

		//Rename src registers
		if(mRNPR[i].rs1 != -1 && mRMT[mRNPR[i].rs1].valid)
		{
			//Get the ROB tag from the RMT
			mRNPR[i].rs1 = mRMT[mRNPR[i].rs1].tag;
			//This value keeps track of whether or not the register has been renamed
			mRNPR[i].rs1ROB = true;
		}
		else
		{
			//This means that the value comes from the ARF
			//So it will be ready once this is dispatched into issue
			mRNPR[i].rs1ROB = false;
		}

		if(mRNPR[i].rs2 != -1 && mRMT[mRNPR[i].rs2].valid)
		{
			//Get the ROB tag and set the flag that this is in the ROB
			mRNPR[i].rs2 = mRMT[mRNPR[i].rs2].tag;
			mRNPR[i].rs2ROB = true;
		}
		else
		{
			//This means that the value comes from the ARF
			mRNPR[i].rs2ROB = false;
		}

		//Update RMT for dest register
		if (mRNPR[i].rd != -1)
		{
			mRMT[mRNPR[i].rd].tag = mRNPR[i].robID;
			mRMT[mRNPR[i].rd].valid = true;
		}
	}

	//Pass along the bundle
	for(int i = 0; i < mWidth; i++) {
		mRRPR[i] = mRNPR[i];
		mRRPR[i].rrEnt = cycle + 1;
		mRRPR[i].rnDur++;
	}
}

void Pipeline::decode() {
	//Increment cycle times if stalled
	if (mDIStall || mRNStall)
	{
		for(int i = 0; i < mWidth; i++) {
			mDEPR[i].deDur++;
		}
		return;
	}

	//Otherwise just pass the bundle along
	for(int i = 0; i < mWidth; i++) {
		mRNPR[i] = mDEPR[i];
		mRNPR[i].rnEnt = cycle + 1;
		mRNPR[i].deDur++;
	}
}

void Pipeline::fetch(instr* input) {
	//Do nothing if stalled
	if (mDIStall || mRNStall)
	{
		return;
	}

	//Pass the bundle along
	for(int i = 0; i < mWidth; i++) {
		mDEPR[i] = input[i];
		mDEPR[i].rs1Orig = input[i].rs1;
		mDEPR[i].rs2Orig = input[i].rs2;
		mDEPR[i].feEnt = cycle;
		mDEPR[i].feDur = 1;
		mDEPR[i].deEnt = cycle+1;
	}
}
