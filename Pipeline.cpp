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
	for(int i = 0; i < mWidth*5; i++)
	{
		mFU[i].cyclesLeft--;
		if(mFU[i].cyclesLeft == 0)
		{

		}
	}

}

void Pipeline::issue() {

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
