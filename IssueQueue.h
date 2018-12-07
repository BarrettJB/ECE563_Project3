/*
 * IssueQueue.h
 *
 *  Created on: Dec 7, 2018
 *      Author: Barrett Bryson
 */

#ifndef ISSUEQUEUE_H_
#define ISSUEQUEUE_H_

class IssueQueue {
public:
	IssueQueue(unsigned long int size);

private:
	bool* mValid;
	unsigned int* mDstTag;
	bool* mRs1Rdy;
	bool* mRs1ROB;
	int* mRs1Val;
	bool* mRs2Rdy;
	bool* mRs2ROB;
	int* mRs2Val;
};

#endif /* ISSUEQUEUE_H_ */
