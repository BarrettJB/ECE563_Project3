/*
 * Pipeline.h
 *
 *  Created on: Dec 7, 2018
 *      Author: Barrett Bryson
 */

#ifndef PIPELINE_H_
#define PIPELINE_H_

typedef struct instr {
	bool valid;
	unsigned long int pc;
	unsigned int op_type;
	int rd;
	unsigned int rs1;
	bool rs1ROB;
	unsigned int rs2;
	bool rs2ROB;
	unsigned int robID;
};

typedef struct rob_entry{
	int dst;
	bool rdy;
	//These two may be unecessary
	bool exc;
	bool miss;
	unsigned long int pc;
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
	bool mDEStall;
	bool mRNStall;

	//Used for wakeup pointers
	bool isTrue;
};

#endif /* PIPELINE_H_ */
