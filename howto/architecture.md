# Architecture

## Temperature/Humidity thread

## Monitoring Thread

@startuml
start
:monitoring=true;
:intrusion in progress=false;

while (monitoring ?)
  if (intrusion in progress ?) then (yes)
  	while (last motion + timeout < now)
  		#CEECF5:read IR sensor;
  		if (motion detected ?) then (yes)
  			:last_motion=now();
                else (no)
  		endif
                :sleep(500ms);
  	endwhile
  	:intrusion in progress = false;
  	#F6CECE:stop the light;
  	#F6CECE:stop image capture;
  	:notify end of intrusion;
  else (no)
  	#CEECF5:read IR sensor;
  	if (motion detected ?) then (yes)
  		:last_motion=now();
  		:intrusion in progress = true;
   		#D8F6CE:start the light;
  		#D8F6CE:start image capture;
  		:notify start of intrusion;
        else (no)
  	endif
  endif

  :sleep(500ms);
endwhile

stop
@enduml

### Image Capture Thread


## Check status
@startuml
start
if (no intrusion in progress ?) then (yes)
	#F6CECE:stop monitoring;
	#D8F6CE:start the light;
	:sleep(200ms);
  	#D8F6CE:capture one image;
  	#F6CECE:stop the light;
  	:notify status;
  	#D8F6CE:start monitoring;
else (no)
	:notify intrusion in progress;
endif

stop
@enduml