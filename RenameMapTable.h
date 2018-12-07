/*
 * RenameMapTable.h
 *
 *  Created on: Dec 7, 2018
 *      Author: Barrett Bryson
 */

#ifndef RENAMEMAPTABLE_H_
#define RENAMEMAPTABLE_H_

class RenameMapTable {
public:
	RenameMapTable();

private:
	bool* mValid;
	unsigned int* mROBTag;
};

#endif /* RENAMEMAPTABLE_H_ */
