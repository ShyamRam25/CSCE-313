#ifndef _BOUNDEDBUFFER_H_
#define _BOUNDEDBUFFER_H_

#include <condition_variable>
#include <mutex>
#include <assert.h>
#include <queue>
#include <vector>
#include <stdio.h>
#include <string.h>


class BoundedBuffer {
private:
    // max number of items in the buffer
	long unsigned int cap;

    /* The queue of items in the buffer
     * Note that each item a sequence of characters that is best represented by a vector<char> for 2 reasons:
	 *  1. An STL std::string cannot keep binary/non-printables
	 *  2. The other alternative is keeping a char* for the sequence and an integer length (b/c the items can be of variable length)
	 * While the second would work, it is clearly more tedious
     */
	std::queue<std::vector<char>> q;

	// add necessary synchronization variables and data structures 
	//mutex
	std::mutex m;
	//2 cond var - one for data available, one for slot available
	std::condition_variable data_available;
	std::condition_variable slot_available;

	

public:
	BoundedBuffer (long unsigned int _cap);
	~BoundedBuffer ();

	void push (char* msg, int size);
	int pop (char* msg, long unsigned size);

	size_t size ();
};

#endif
