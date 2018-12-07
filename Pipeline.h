/*
 * Pipeline.h
 *
 *  Created on: Dec 7, 2018
 *      Author: Barrett Bryson
 */

#ifndef PIPELINE_H_
#define PIPELINE_H_

typedef struct rob_entry{
	int value;
	unsigned int dst;
	bool rdy;
	bool exc;
	bool miss;
	unsigned long int pc;
};

typedef struct iq_entry{
	bool valid;
	unsigned int dstTag;
	bool rs1Rdy;
	bool rs1ROB;
	int rs1Val;
	bool rs2Rdy;
	bool rs2ROB;
	int rs2Val;
};

typedef struct rmt_entry {
	bool valid;
	unsigned int tag;
};


typedef struct instr {
	bool valid;
	unsigned long int pc;
	unsigned int op_type;
	int rd;
	unsigned int r1;
	unsigned int r2;
	unsigned int robID;
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
	void fetch();

private:
	unsigned long int mWidth;
	int* mARF;
	rob_entry* mROB;
	unsigned int mROBsize;
	unsigned int mHead;
	unsigned int mTail;
	iq_entry* mIQ;
	rmt_entry* mRMT;


	//Pipeline Registers
	instr* mDEPR;
	instr* mRNPR;
	instr* mRRPR;
	instr* mDIPR;
	fu* mFU;
	instr* mWBPR;
};

#endif /* PIPELINE_H_ */
