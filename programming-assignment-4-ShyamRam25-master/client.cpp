#include <fstream>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include <sys/wait.h>

#include "BoundedBuffer.h"
#include "common.h"
#include "Histogram.h"
#include "HistogramCollection.h"
#include "TCPRequestChannel.h"

// ecgno to use for datamsgs
#define EGCNO 1

using namespace std;

void patient_thread_function (int p_no, int num_threads, BoundedBuffer& request_buffer) {
    // functionality of the patient threads

    //take a patient p_no; 
    // for n requests, produce a datamsg(p_no, time, ECGNO) and push to request_buffer
    //     -- time dependent on current requests
    //     -- at 0 -> time = 0.00; at 1 -> time = 0.004; at 2 -> time = 0.008
    for (int i = 0; i < num_threads; i++) {
        datamsg msg(p_no, 0.004*i, EGCNO);
        request_buffer.push((char*) &msg, sizeof(datamsg));
    }
}

void file_thread_function (/* add necessary arguments */int filesize, string fn, BoundedBuffer& request_buffer, int m) {
    // functionality of the file thread

    //file size (00 appended with file name)
    //open output file; allocate the memory with fseek; close the file
    string filename = fn;
    string temp = "received/" + fn;

    FILE* fp = fopen(temp.c_str(), "w+");
    fseek(fp, filesize, SEEK_SET);
    fclose(fp);
    
    for (int i = 0; i <= filesize / m; i ++) {
        filemsg msg(0, 0);
        msg.length = m;
        int offset = i * m;
        msg.offset = offset;

        int remainder = filesize - offset;
        if (remainder < m) {
            msg.length = remainder;
        }
        int length = sizeof(filemsg) + filename.size() + 1;
        char * buf = new char[length];
        
        memcpy(buf, &msg, sizeof(filemsg));
        strcpy(buf + sizeof(filemsg), filename.c_str());

        request_buffer.push(buf, length);
        delete[] buf;
    }

    //while offset < file_size, produce a filemsg(offset, m) + filename and push to request_buffer
    //       - incrementing offset; and be careful with the final message
}

void worker_thread_function (TCPRequestChannel* chan, BoundedBuffer& request_buffer, BoundedBuffer& response_buffer, string filename, int m) {
    // functionality of the worker threads


    std::vector<char> buf2(m);
    // forever loop
    while (true) {
        char buf [MAX_MESSAGE];
        // pop message from request buffer
        request_buffer.pop(buf, sizeof(buf));

        MESSAGE_TYPE message = *((MESSAGE_TYPE*) buf);
        // view line 120 in server (process_request function) for how to decide current message

        // send the message across the FIFO channel
        // collect response
        if (message == QUIT_MSG) {
            chan->cwrite(&message, sizeof(MESSAGE_TYPE));
            break;
        }
        // if DATA:
        //    - create pair of p_no from message and response from server
        //    - push pair to the response_buffer
        if (message == DATA_MSG) {
            datamsg* p = (datamsg*)buf;

            chan->cwrite(buf, sizeof(buf));

            double response;
            chan->cread(&response, sizeof(double));

            std::pair<int,double> temp = std::pair<int,double>(p->person, response);
            response_buffer.push((char*) &temp, sizeof(temp));
        }
        // if FILE:
        //   
        
        //    - fseek(SEEK_SET) to offset the filemsg
        //    - write the buffer from the server
        else if (message == FILE_MSG) {
            // collect the filename from the message
            filemsg * f = ((filemsg*) buf);
            string new_filename = "received/" + filename;

            //    - open the file in update mode
            FILE* fp = fopen(new_filename.c_str(), "r+");
            fseek(fp, f->offset, SEEK_SET);
            
            int filesize = new_filename.size() + 1 + sizeof(filemsg);
            chan->cwrite(buf, filesize);
           
            char * response = new char[m];
            chan->cread(response, f->length);

            fwrite(response, 1, f->length, fp);
            fclose(fp);
            delete [] response;
        }
    }

   

    

    
    
}

void histogram_thread_function (/* add necessary arguments */HistogramCollection& hc, BoundedBuffer& response_buffer) {

    while (true) {
        char * buf = new char[MAX_MESSAGE];
        response_buffer.pop(buf, MAX_MESSAGE);
        std::pair<int, double>* pair = (std::pair<int, double>*)buf;

        if (pair->first == -1 && pair->second == -1) {
            delete[] buf;
            break;
        }
        hc.update(pair->first, pair->second);
        delete[] buf;
    }

}


int main (int argc, char* argv[]) {
    int n = 1000;	// default number of requests per "patient"
    int p = 10;		// number of patients [1,15]
    int w = 100;	// default number of worker threads
	int h = 20;		// default number of histogram threads
    int b = 20;		// default capacity of the request buffer (should be changed)
	int m = MAX_MESSAGE;	// default capacity of the message buffer
	string f = "";	// name of file to be transferred
    string a = "127.0.0.1";
    string r = "800";
    
    // read arguments
    int opt;
	while ((opt = getopt(argc, argv, "n:p:w:h:b:m:f:a:r:")) != -1) {
		switch (opt) {
			case 'n':
				n = atoi(optarg);
                break;
			case 'p':
				p = atoi(optarg);
                break;
			case 'w':
				w = atoi(optarg);
                break;
			case 'h':
				h = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
                break;
			case 'm':
				m = atoi(optarg);
                break;
			case 'f':
				f = optarg;
                break;
            case 'a':
                a = optarg;
                break;
            case 'r':
                r = optarg;
                break;
		}
	}
    
    /*
	// fork and exec the server
    int pid = fork();
    if (pid == 0) {
        execl("./server", "./server", "-m", (char*) to_string(m).c_str(), nullptr);
    }
    */
    
	// initialize overhead (including the control channel)
    TCPRequestChannel* chan = new TCPRequestChannel(a, r);
    BoundedBuffer request_buffer(b);
    BoundedBuffer response_buffer(b);
	HistogramCollection hc;

    //array of producer threads (if data, p elements: if file, 1 element)
    vector<thread> producer_threads;
    //array of FIFOs (w elements)
    vector<TCPRequestChannel*> channels;
    //array of worker threads (w elements)
    vector<thread> worker_threads;
    //array of histogram threads (if data, h elements: if file, 0 elements)
    vector<thread> histogram_threads;

    // making histograms and adding to collection
    for (int i = 0; i < p; i++) {
        Histogram* h = new Histogram(10, -2.0, 2.0);
        hc.add(h);
    }
	
	// record start time
    struct timeval start, end;
    gettimeofday(&start, 0);

    /* create all threads here */
    // DO THIS METHOD BELOW -----------
    //if data:
    // create p patient threads

    // if data:
    if (f == "") {
        for (int i = 0; i < p; i++) {
            producer_threads.push_back(thread(patient_thread_function, i + 1, n, std::ref(request_buffer)));
        }
        // create w worker threads
        //      - create w channels
        for (int i = 0; i < w; i++) {
            /*
            MESSAGE_TYPE nc = NEWCHANNEL_MSG;
            chan->cwrite(&nc, sizeof(MESSAGE_TYPE));

            char buf[MAX_MESSAGE];
            chan->cread(buf, MAX_MESSAGE);

            string name(buf);
            */
            TCPRequestChannel* temp = new TCPRequestChannel(a, r);
            channels.push_back(temp);

            worker_threads.push_back(thread(worker_thread_function, channels.at(i), std::ref(request_buffer), std::ref(response_buffer), f, m));
            //AND THIS ONE SUCKS
        }

        //if data:
        // create h histogram threads
        for (int i = 0; i < h; i ++) {
            histogram_threads.push_back(thread(histogram_thread_function, std::ref(hc), std::ref(response_buffer)));
        }
    }
    else { //if file: 
        // if file:
        // create 1 file thread
        filemsg fmsg(0, 0);
        int length = sizeof(filemsg) + f.size() + 1;
        char * temp = new char [length];

        memcpy(temp, &fmsg, sizeof(fmsg));
        strcpy(temp + sizeof(fmsg), f.c_str());
        chan->cwrite(temp, length);
       
        int64_t filesize = 0;
        chan->cread(&filesize, sizeof(int64_t));
        
        producer_threads.push_back(thread(file_thread_function, filesize, f, std::ref(request_buffer), m));
        //AND MAYBE THIS ONE SUCKS
        delete[] temp;
        for (int i = 0; i < w; i++) {
            /*
            MESSAGE_TYPE nc = NEWCHANNEL_MSG;
            chan->cwrite(&nc, sizeof(MESSAGE_TYPE));

            char buf[MAX_MESSAGE];
            chan->cread(buf, MAX_MESSAGE);

            string name(buf);
            */
            TCPRequestChannel* temp = new TCPRequestChannel(a, r);
            channels.push_back(temp);

            worker_threads.push_back(thread(worker_thread_function, channels.at(i), std::ref(request_buffer), std::ref(response_buffer), f, m));
            //AND THIS ONE SUCKS
        }
    }
    
	/* join all threads here */
    // iterate over all thread arrays, calling join
    for (long unsigned int i = 0; i < producer_threads.size(); i++) {
        producer_threads[i].join();
    }

    for (long unsigned int i = 0; i < worker_threads.size(); i++) {
        MESSAGE_TYPE q = QUIT_MSG;
        request_buffer.push((char*) &q, sizeof(MESSAGE_TYPE));
    }
    for (long unsigned int i = 0; i < worker_threads.size(); i++) {
        worker_threads[i].join();
    }

    for (long unsigned int i = 0; i < histogram_threads.size(); i++) {
        std::pair<int, double> temp(-1, -1.0);
        response_buffer.push((char*) &temp, sizeof(temp));
    }
    for (long unsigned int i = 0; i < histogram_threads.size(); i++) {
        histogram_threads[i].join();
    }

	// record end time
    gettimeofday(&end, 0);

    // print the results
	if (f == "") {
		hc.print();
	}
    int secs = ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) / ((int) 1e6);
    int usecs = (int) ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) % ((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;

    //quit and close all channels in FIFO array
	// quit and close control channel
    MESSAGE_TYPE q = QUIT_MSG;

    for (long unsigned int i = 0; i < channels.size(); i++) {
        channels[i]->cwrite((char*) &q, sizeof(MESSAGE_TYPE));

        delete channels[i];
    }

    chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
    cout << "All Done!" << endl;
    delete chan;

	// wait for server to exit
	wait(nullptr);
}
