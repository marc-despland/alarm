#ifndef ILIGHTCONTROLLER_H
#define ILIGHTCONTROLLER_H


class ILightController {
	public:
		virtual void lightOn()=0;
		virtual void lightOff()=0;
		virtual bool togglePause()=0;
};

#endif