#ifndef _CONNECTION_H
#define _CONNECTION_H
#include "host.h"

class Connection {
public:
	Connection(int socketfd, Host * host);
	Host * endpoint();
	int socket();
	friend std::ostream & operator<<(std::ostream &os, const Connection * c);

private:
	int socketfd;
	Host * host;
};

#endif