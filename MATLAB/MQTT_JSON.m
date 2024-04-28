myMQTT = mqttclient('tcp://mqtt.eclipseprojects.io', Port = 1883);
Topic_sub = "Test";

filePath = '"D:\NCKH\CODE\Human-Activity-Recognition\MQTT2FIREBASE.py"';

% Tự động gọi và chạy file thông qua cmd
command = ['cmd /c start "" python "', filePath, '"'];
status = system(command);

% Kiểm tra xem việc gọi lệnh có thành công hay không
if status == 0
    disp('Command executed successfully.');
else
    disp('Error executing command.');
end

ax = 0;
ay = 0;
az = 0;
gx = 0;
gy = 0;
gz = 0;
global temp;
temp = 0;


data_acc = struct('ax', ax, 'ay', ay, 'az', az);
data_gy = struct('gx', gx, 'gy', gy, 'gz', gz);
hfig = figure;clf
hax = axes(hfig);



anim_lineaX = animatedline('Parent',hax,'Color','red','MaximumNumPoints',60);
anim_lineaY = animatedline('Parent',hax,'Color','green','MaximumNumPoints',60);
anim_lineaZ = animatedline('Parent',hax,'Color','blue','MaximumNumPoints',60);


legend('Ax', 'Ay', 'Az');

gfig = figure;clf
hgy = axes(gfig);
anim_linegX = animatedline('Parent',hgy,'Color','red','MaximumNumPoints',60);
anim_linegY = animatedline('Parent',hgy,'Color','green','MaximumNumPoints',60);
anim_linegZ = animatedline('Parent',hgy,'Color','blue','MaximumNumPoints',60);
legend('Gx', 'Gy', 'Gz');

Data = subscribe(myMQTT,Topic_sub, QualityOfService = 1, Callback = @(src, msg) handleMessage(src, msg, anim_lineaX, anim_lineaY, anim_lineaZ, anim_linegX, anim_linegY, anim_linegZ));

function handleMessage(~, message, anim_lineaX, anim_lineaY, anim_lineaZ, anim_linegX, anim_linegY, anim_linegZ)
    try
        tic;
        op = jsondecode(message);
        disp(op);

        ax = op.ax;
        ay = op.ay;
        az = op.az;
        %data_acc.ax = [data.ax, ax];
        %data_acc.ay = [data.ay, ay];
        %data_acc.az = [data.az, az];
        gx = op.gx;
        gy = op.gy;
        gz = op.gz;
        %data_gy.gx = [data.gx, gx];
        %data_gy.gy = [data.gy, gy];
        %data_gy.gz = [data.gz, gz];
        global temp;
        t = op.Time;
        disp(['t ',num2str(t)]);
        disp(['temp ',num2str(temp)]);
        if t<=temp
            disp('Return');
            return
        end

        %subplot(2,1,1)
        addpoints(anim_lineaX,t,ax);
        addpoints(anim_lineaY,t,ay);
        addpoints(anim_lineaZ,t,az);
        %plot(data_acc)
     
        %subplot(2,1,2)
        addpoints(anim_linegX,t,gx);
        addpoints(anim_linegY,t,gy);
        addpoints(anim_linegZ,t,gz);
        %plot(data_gy)
        temp = t;
        elapsedTime = toc;
        disp(['Thời gian chạy của callback là: ', num2str(elapsedTime), ' giây']);

    catch ME
        disp(['Lỗi khi xử lý tin nhắn: ' ME.message]);
    end
end