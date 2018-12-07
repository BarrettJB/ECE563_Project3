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

	mROB = new ReorderBuffer(rob_size);
	mIQ = new IssueQueue(iq_size);
	mRMT = new RenameMapTable(67);
}

