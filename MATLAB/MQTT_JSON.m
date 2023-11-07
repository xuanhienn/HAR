myMQTT = mqttclient('tcp://mqtt.eclipseprojects.io', Port = 1883);
Topic_sub = "Test";

Data = subscribe(myMQTT,Topic_sub);

x=0;
y=0;
z=0;
values_check = -1;
while true
   messMqtt = read(myMQTT, Topic = "Test");
   if ~isempty(messMqtt)
        op = jsondecode(messMqtt.Data(1));
        values = op.Time;
        if(values ~= values_check)
            temp = op.temp;
            hum = op.humidity;
            ran = op.Random;
      
            x = [x,temp];
            y = [y,hum];
            z = [z,ran];
      
            subplot(3,1,1);
            plot(x);
            ylim([0,10]);
            grid ON;
            title("Temperature");
      
            subplot(3,1,2);
            plot(y);
            ylim([0,10]);
            grid ON;
            title("Humidity");
      
            subplot(3,1,3);
            plot(z);
            ylim([0,10]);
            grid ON;
            title("Random Number");
            values_check = values_check + 1;
            drawnow     
        end
   end   
   
end
