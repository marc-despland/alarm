#ifndef _FIFO_H
#define _FIFO_H

#include <mutex>
#include <queue>
#include <condition_variable>
using namespace std;

class Fifo {
public:
	Fifo();
	void add(int socket);
	int next();
	int size();
	void unlock();

private:
	bool unlocked;
	std::mutex read;
	std::mutex write;
	std::condition_variable ready;
	std::queue<int> fifo;

};

#endif