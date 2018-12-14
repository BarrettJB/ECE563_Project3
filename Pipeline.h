/*
 * Pipeline.h
 *
 *  Created on: Dec 7, 2018
 *      Author: Barrett Bryson
 */

#ifndef PIPELINE_H_
#define PIPELINE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct instr {
	bool valid;
	unsigned long int pc;
	unsigned int op_type;
	int rd;
	int rs1;
	bool rs1ROB;
	int rs2;
	bool rs2ROB;
	unsigned int robID;

	//Used for validation printout;
	int feEnt;
	int feDur;
	int deEnt;
	int deDur;
	int rnEnt;
	int rnDur;
	int rrEnt;
	int rrDur;
	int diEnt;
	int diDur;
	int isEnt;
	int isDur;
	int exEnt;
	int exDur;
	int wbEnt;
	int wbDur;
	int rtEnt;
	int rtDur;
	int rs1Orig;
	int rs2Orig;
	int traceLine;
};

typedef struct rob_entry{
	int dst;
	bool rdy;
	//These two may be unecessary
	bool exc;
	bool miss;
	unsigned long int pc;
	instr robInstr;
};

typedef struct iq_entry{
	instr iqInstr;
	int age;
	bool valid;
	unsigned int dstTag;
	bool *rs1Rdy;
	bool *rs2Rdy;
};

typedef struct rmt_entry {
	bool valid;
	unsigned int tag;
};

typedef struct fu {
	bool inUse;
	unsigned int cyclesLeft;
	instr fuInstr;
};


class Pipeline {
public:
	Pipeline(unsigned long int width, unsigned long int iq_size, unsigned long int rob_size);
	void retire();
	void writeback();
	void execute();
	void issue();
	void dispatch();
	void regRead();
	void rename();
	void decode();
	void fetch(instr* input);

	//Used to end the process once all instructions retire
	bool eof;
	bool finished;

	int cycle;

private:
	unsigned long int mWidth;
	int* mARF;
	rob_entry* mROB;
	unsigned int mROBsize;
	unsigned int mHead;
	unsigned int mTail;
	iq_entry* mIQ;
	unsigned int mIQsize;
	unsigned int mIQentries;
	rmt_entry* mRMT;


	//Pipeline Registers
	instr* mDEPR;
	instr* mRNPR;
	instr* mRRPR;
	instr* mDIPR;
	fu* mFU;
	instr* mWBPR;


	//Used to stall early processes depending on availability
	bool mDIStall;
	bool mRNStall;

	//Used for wakeup pointers
	bool isTrue;

};

#endif /* PIPELINE_H_ */
