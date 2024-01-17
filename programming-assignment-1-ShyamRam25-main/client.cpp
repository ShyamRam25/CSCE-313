/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name: Shyam Ramachandran
	UIN: 931009307
	Date: 09/07/23
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include<sys/wait.h>
#include <iostream>


using namespace std;


int main (int argc, char *argv[]) {
	int opt;
	int p = -1; // Maybe needs to be -1
	double t = -1; // Maybe needs to be -1
	int e = -1;
	int f = -1;
	int m = MAX_MESSAGE;
	bool new_chan = false;
	vector<FIFORequestChannel*> channels;
	
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				f = 1;
				filename = optarg;
				break;
			case 'm':
				m = atoi(optarg);
				break;
			case 'c':
				new_chan = true;
				break;
		}
	}

	pid_t server_process_id = fork();

	if (server_process_id == 0) {
		//In the child process
		//Create an array containing the executable and the arguments
		
		
		string new_m = to_string(m);
		vector <char> m_vector;

		for (size_t i = 0; i < new_m.size(); i++) {
			m_vector.push_back(new_m[i]);
		}

		char * args [] = {(char *)("./server"), (char *)("-m"), &m_vector[0], nullptr};
		execvp(args[0], args); //Maybe do error checking
	}

    FIFORequestChannel cont_chan("control", FIFORequestChannel::CLIENT_SIDE);

	channels.push_back(&cont_chan);

	if (new_chan) {
		//send newchannel request to server
		MESSAGE_TYPE nc = NEWCHANNEL_MSG;
    	cont_chan.cwrite(&nc, sizeof(MESSAGE_TYPE));
		//create a variable to hold the name
		char buffer[MAX_MESSAGE];
		//cread the response from the server
		
		cont_chan.cread(buffer, sizeof(datamsg)); //NEED TO ITERATE THROUGH AND SEE IF ITS NULL
		//call the FIFOChannelConstructor with the name from the server
		string servername(buffer);

		FIFORequestChannel * newestchan = new FIFORequestChannel(servername, FIFORequestChannel::CLIENT_SIDE); //WHAT TO PUT HERE
		//Push the new channel into the vector
		channels.push_back(newestchan);
	}

	FIFORequestChannel chan = *(channels.back());
		//WHERE DOES THIS GO TODO
	
	//Single datapoint, only run if p, t, e != .1
	if ((p != -1) and (t != -1) and (e != -1)) {

		char buf[MAX_MESSAGE]; // 256
		datamsg x(p, t, e); // Change from Hardcoding to User's value's
		memcpy(buf, &x, sizeof(datamsg));
		chan.cwrite(buf, sizeof(datamsg)); // question
		double reply;
		chan.cread(&reply, sizeof(double)); //answer
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	}
	else if (p != -1) {

		ofstream file;
		file.open("./received/x1.csv");

		for (int i = 0; i < 1000; i++) {
		
			double time = i * 0.004;
			// double ecg1 = 0;
			// double ecg2 = 0;

			char buf[MAX_MESSAGE];
			datamsg x(p, time, 1); //MAYBE CHECK THIS
			
			memcpy(buf, &x, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg)); // question
			double reply;
			chan.cread(&reply, sizeof(double)); //answer
			// ecg1 = reply;


			char buf2[MAX_MESSAGE];
			datamsg x2(p, time, 2); //MAYBE CHECK THIS
			memcpy(buf2, &x2, sizeof(datamsg));
			chan.cwrite(buf2, sizeof(datamsg));
			double reply2;
			chan.cread(&reply2, sizeof(double));
			// ecg2 = reply2;
			
			file << time << "," << reply << "," << reply2 << endl;
		}
		file.close();
	}

	// Else if p != .1, request 1000 datapoints
	//Loop over 1st 1000 lines
	//send request for ecg 1
	//send request for ecg 2
	//write line to recieved/x1.csv
	
	//STEP 3: FILE REQUEST ----

    // sending a non-sense message, you need to change this

	if (f != -1) {

		filemsg fm(0, 0);
		string fname = filename;
		
		int len = sizeof(filemsg) + (fname.size() + 1);
		char* buf2 = new char[len];
		memcpy(buf2, &fm, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), fname.c_str());
		chan.cwrite(buf2, len);  // I want the file length;

		int64_t filesize = 0;
		chan.cread(&filesize, sizeof(int64_t));


		char* buf3 = new char[m];//Create buffer of size buff capacity(m) TODO

		//Loop over the segments in the file filesize / buff capacity(m)
		//create filemsg instance
		ofstream file2;
		file2.open("./received/" + filename);

		int temp = m;

		int remaining_bytes = filesize; // IS THIS RIGHT??

		while (remaining_bytes > 0) {
			m = temp;

			if (remaining_bytes < m) {
				m = remaining_bytes;
			}
			int64_t offset = filesize - remaining_bytes;
			filemsg* file_req = (filemsg*) buf2;
			file_req->offset = offset; //set the offset in the file
			file_req->length = m;//set the length. Be careful of the last segment

			//send the request (buf2)
			chan.cwrite(buf2, len);
			//recieve the response
			//cread into buf3 length file_req->len
			chan.cread(buf3, file_req->length);
			
			//write buf3 into file: recieved/filename
			file2.write(buf3, file_req->length);

			remaining_bytes -= m;
		}

		/*
		for (int i = 0; i <= filesize / m; i++) { //MAYBE <=
			int remaining_bytes = filesize - (m * i); //NOT RIGHT??
			filemsg* file_req = (filemsg*) buf2;
			file_req->offset = i * m; //set the offset in the file
			file_req->length = m;//set the length. Be careful of the last segment

			if (remaining_bytes < m) {
				file_req->length = remaining_bytes;
			}

			//send the request (buf2)
			chan.cwrite(buf2, len);
			//recieve the response
			//cread into buf3 length file_req->len
			chan.cread(buf3, file_req->length);
			
			//write buf3 into file: recieved/filename
			file2.write(buf3, file_req->length);
		}
		*/

		file2.close();
		delete[] buf2;
		delete[] buf3;
	}

	//... start timer

	/*
	remaining_bytes = filesize
	while (remaining_bytes is not 0):

		if the remaining bytes is less than buffer size:
			length = remaining_bytes
		offset = filesize - remaining_bytes

		make a file transfer
		filemsg(offset, length)
	*/
	


	//...stop timer

	//If necessary, close and delete the new channel

	
	// closing the channel    
    
	
	if (new_chan) {
		//Do your close and deletes

		MESSAGE_TYPE quit = QUIT_MSG;
		for (long unsigned int i = 0; i < channels.size(); i++) {
		channels[i]->cwrite(&quit, sizeof(MESSAGE_TYPE));
		}
		delete channels.back();
	}

	MESSAGE_TYPE mm = QUIT_MSG;
	cont_chan.cwrite(&mm, sizeof(MESSAGE_TYPE));
	wait(NULL);
	return 0;
}
