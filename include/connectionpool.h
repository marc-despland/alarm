#ifndef _CONNECTIONPOOL_H
#define _CONNECTIONPOOL_H

#include <vector>
#include <map>
#include <mutex>
#include <sys/epoll.h>

#include "connection.h"


using namespace std;


class ConnectionPoolException: public exception {
	public:
		ConnectionPoolException(string message):exception() {
			this->message=message;
		}
		~ConnectionPoolException() throw (){};
		friend std::ostream & operator<<(std::ostream &os, const ConnectionPoolException& e) {
			return os << e.message;
		}
	protected:
		string message;
};



class ConnectionPool {

public:
	ConnectionPool()throw (ConnectionPoolException);
	void add(int socket)throw (ConnectionPoolException);
	int next(int socket);
	vector<int> * poll();
	//void close(int socket);

private:
	vector<int> cntxArray;
	map<int,int> cntxMap;
	mutex	safe;
	int		pool;
	struct epoll_event *events;
};

#endif