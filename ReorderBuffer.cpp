/*
 * ReorderBuffer.cpp
 *
 *  Created on: Dec 7, 2018
 *      Author: Barrett Bryson
 */

#include "ReorderBuffer.h"

ReorderBuffer::ReorderBuffer(unsigned long int size) {
	mValue = new int[size];
	mDst = new unsigned int[size];
	mRdy = new bool[size];
	mExc = new bool[size];
	mMiss = new bool[size];
	mPC = new unsigned long int[size];

	mHead = 0;
	mTail = 0;
}

