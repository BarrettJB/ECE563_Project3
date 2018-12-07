/*
 * IssueQueue.cpp
 *
 *  Created on: Dec 7, 2018
 *      Author: Barrett Bryson
 */

#include "IssueQueue.h"

IssueQueue::IssueQueue(unsigned long int size) {
	mValid = new bool[size];
	mDstTag = new unsigned int[size];
	mRs1Rdy = new bool[size];
	mRs1ROB = new bool[size];
	mRs1Val = new int[size];
	mRs2Rdy = new bool[size];
	mRs2ROB = new bool[size];
	mRs2Val = new int[size];
}

