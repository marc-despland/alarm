#include "host.h"

Host::Host(string name, unsigned int port) {
	this->myname=name;
	this->myport=port;
}
Host::~Host() {
}

unsigned int Host::port() {
	return this->myport;
}
string Host::name() {
	return this->myname;
}


std::ostream & operator<<(std::ostream &os, const Host& h) {
	return os << h.myname<<":"<<h.myport;
}

std::ostream & operator<<(std::ostream &os, const Host * h) {
	return os << h->myname<<":"<<h->myport;
}
