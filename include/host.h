#ifndef _HOST_H
#define _HOST_H

#include <iostream>
#include <string>

using namespace std;


class Host {
	public:
		Host(string name, unsigned int port);
		virtual ~Host();
		friend std::ostream & operator<<(std::ostream &os, const Host& h);
		friend std::ostream & operator<<(std::ostream &os, const Host * h);
		
		virtual unsigned int port();
		virtual string name();
	protected:
		string myname;
		unsigned int myport;
};		

#endif