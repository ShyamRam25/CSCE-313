#include "BoundedBuffer.h"

using namespace std;


BoundedBuffer::BoundedBuffer (long unsigned int _cap) : cap(_cap) {
    // modify as needed
}

BoundedBuffer::~BoundedBuffer () {
    // modify as needed
}

void BoundedBuffer::push (char* msg, int size) {
    unique_lock<mutex> lock(m);
    // 1. Convert the incoming byte sequence given by msg and size into a vector<char>
    vector<char> v(msg, msg + size);

    // 2. Wait until there is room in the queue (i.e., queue lengh is less than cap)
    //      waiting on slot available

    slot_available.wait(lock, 
    [this]{ return q.size() < cap; });

    // 3. Then push the vector at the end of the queue
    q.push(v);

    // 4. Wake up threads that were waiting for push
    //      notifying data available  
    lock.unlock();
    data_available.notify_one();

}

int BoundedBuffer::pop (char* msg, long unsigned int size) {
    unique_lock<mutex> lock(m);
    // 1. Wait until the queue has at least 1 item
    //      waiting on data available
    data_available.wait(lock, 
    [this]{ return q.size() != 0; });

    // 2. Pop the front item of the queue. The popped item is a vector<char>
    vector<char> v = q.front();
    q.pop();
    lock.unlock();

    // 3. Convert the popped vector<char> into a char*, copy that into msg; assert that the vector<char>'s length is <= size
    //      use vector::data()
    assert(v.size() <= size);
    memcpy(msg, v.data(), v.size());



    // 4. Wake up threads that were waiting for pop
    //    notifying slot available
    slot_available.notify_one();

    // 5. Return the vector's length to the caller so that they know how many bytes were popped
    return v.size();
}

size_t BoundedBuffer::size () {
    return q.size();
}
