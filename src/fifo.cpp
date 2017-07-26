#include "fifo.h"
#include "log.h"

Fifo::Fifo()  { 
	this->unlocked=false;
}

void Fifo::unlock() {
	this->unlocked=true;
	this->ready.notify_all();
}

void Fifo::add(int socket) {
	//this->write.lock();
	//std::unique_lock<std::mutex> lck(this->read);
	{
		std::lock_guard<std::mutex> lock(this->read);
		this->fifo.push(socket);
	}
	Log::logger->log("Fifo",DEBUG) << "Adding element " << socket<<endl;
	//this->write.unlock();
	this->ready.notify_one();
}

int Fifo::next() {
	Log::logger->log("Fifo",DEBUG) << "Next" <<endl;
	std::unique_lock<std::mutex> lck(this->read);
	Log::logger->log("Fifo",DEBUG) << "After unique lock" <<endl;
	while (this->fifo.empty()) {
		this->ready.wait(lck);
		if (this->unlocked) return -1;
	}
	int tmp=this->fifo.front();
	this->fifo.pop();
	Log::logger->log("Fifo",DEBUG) << "Next socket " << tmp<<endl;
	return tmp;
}

 
int Fifo::size() {
	int tmp=this->fifo.size();
	return tmp;
}


