/*
 * ReorderBuffer.h
 *
 *  Created on: Dec 7, 2018
 *      Author: Barrett Bryson
 */

#ifndef REORDERBUFFER_H_
#define REORDERBUFFER_H_

class ReorderBuffer {
public:
	ReorderBuffer(unsigned long int size);

private:
	int* mValue;
	unsigned int* mDst;
	bool* mRdy;
	bool* mExc;
	bool* mMiss;
	unsigned long int* mPC;

	int mHead;
	int mTail;

};

#endif /* REORDERBUFFER_H_ */
