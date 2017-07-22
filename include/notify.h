#ifndef NOTIFY_H
#define NOTIFY_H
using namespace std;

#define START_INTRUSION 1
#define STOP_INTRUSION 2


class Notify {
	public:
		static string ApiKey;
		static string ApiUrl;
		static void notify(int message);
	protected:
		static void *send(void *message);
};
#endif