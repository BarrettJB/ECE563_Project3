/*
 * Pipeline.h
 *
 *  Created on: Dec 7, 2018
 *      Author: Barrett Bryson
 */

#ifndef PIPELINE_H_
#define PIPELINE_H_

#include "ReorderBuffer.h"
#include "IssueQueue.h"
#include "RenameMapTable.h"


class Pipeline {
public:
	Pipeline(unsigned long int width, unsigned long int iq_size, unsigned long int rob_size);

private:
	unsigned long int mWidth;
	int* mARF;
	RenameMapTable mRMT;
	ReorderBuffer mROB;
	IssueQueue mIQ;

};

#endif /* PIPELINE_H_ */
