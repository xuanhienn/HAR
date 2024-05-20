myMQTT = mqttclient('tcp://mqtt.eclipseprojects.io', Port = 1883);
Topic_sub = "Test";

filePath = 'D:\NCKH\CODE\Human-Activity-Recognition\MQTT2FIREBASE.py';

command = ['cmd /c start "" python "', filePath, '"'];
status = system(command);

if status == 0
    disp('Command executed successfully.');
else
    disp('Error executing command.');
end
act = "";
ax = 0;
ay = 0;
az = 0;
gx = 0;
gy = 0;
gz = 0;
global temp;
temp = 0;
global buff;
buff = zeros(1,100,6);
global reuse_buff;
reuse_buff = zeros(1,60,6);
global params;
params= importONNXFunction("D:\NCKH\CODE\Human-Activity-Recognition\MATLAB\model_onnx2_v3.onnx","modelHAR3");


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
        ax = op.ax;
        ay = op.ay;
        az = op.az;
        gx = op.gx;
        gy = op.gy;
        gz = op.gz;
        act = op.activity;
        global temp;
        t = op.Time;
        if t<=temp
            return
        end

        addpoints(anim_lineaX,t,ax);
        addpoints(anim_lineaY,t,ay);
        addpoints(anim_lineaZ,t,az);

        addpoints(anim_linegX,t,gx);
        addpoints(anim_linegY,t,gy);
        addpoints(anim_linegZ,t,gz);

        temp = t;

        global buff;
        global params;
        global reuse_buff;
        nextIndex = find(buff(1,:,1) == 0, 1);
        buff(1, nextIndex, :) =  [ax, ay, az, gx, gy, gz];
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %disp("flag1");
        data_2d = reshape(buff, 100, 6);
        %disp("flag2");
        segment_length = 60;
        overlap_length = 40;
        %disp("flag3");
        step_size = segment_length - overlap_length;
        %disp("flag4");
        num_buffers = ceil((size(data_2d, 1) - overlap_length) / step_size);
        %disp("flag5");
        buffered_data = zeros(num_buffers, segment_length, size(data_2d, 2));
        %disp("flag6");
        for k = 1:num_buffers
            start_idx = (k - 1) * step_size + 1;
            %disp("flag7");
            end_idx = start_idx + segment_length - 1;
            %disp("flag8");
            if end_idx <= size(data_2d, 1)
                buffered_data(k, :, :) = data_2d(start_idx:end_idx, :);
                %disp("flag9");
            else
                % Handle cases where the buffer would exceed data length
                buffered_data(k, 1:(size(data_2d, 1) - start_idx + 1), :) = data_2d(start_idx:end, :);
                %disp("flag10");
            end
        end
        buffered_data_3d = reshape(buffered_data, num_buffers, segment_length, size(buff, 3));
        %buffered_data_3d = reshape(buffered_data, 3, 60, 6);
        %disp("flag11");
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %disp("flagn");
        %disp(nextIndex);
        if nextIndex >= 100
           %disp("flag model 1")
           [dense_1, ] = modelHAR3(buffered_data_3d(3,1:60,:), params);
           [~, predicted_class] = max(dense_1);
           predicted_label = ["jogging","sitting","stairs","standing","walking"];
           disp(["Predict Activity: " predicted_label(predicted_class)])
           disp(['Real act: ' act]);
           %disp(num2str(predicted_class));
           reuse_buff = buffered_data_3d(3,1:60,:);
           buff = zeros(1, 100, 6);
        elseif nextIndex >= 80
           %disp("flag model 2")
           [dense_1, ] = modelHAR3(buffered_data_3d(2,1:60,:), params);
           [~, predicted_class] = max(dense_1);
           predicted_label = ["jogging","sitting","stairs","standing","walking"];
           disp(["Predict Activity: " predicted_label(predicted_class)])
           disp(['Real act: ' act]);
           %disp(num2str(predicted_class));
        elseif nextIndex >= 60
           %disp("flag model 3")
           [dense_1, ] = modelHAR3(buffered_data_3d(1,1:60,:), params);
           [~, predicted_class] = max(dense_1);
           predicted_label = ["jogging","sitting","stairs","standing","walking"];
           disp(["Predict Activity: " predicted_label(predicted_class)])
           disp(['Real act: ' act]);
           %disp(num2str(predicted_class));
        elseif nextIndex >= 0
           %disp("flag model 4")
           [dense_1, ] = modelHAR3(reuse_buff, params);
           [~, predicted_class] = max(dense_1);
           predicted_label = ["jogging","sitting","stairs","standing","walking"];
           disp(["Predict Activity: " predicted_label(predicted_class)])
           disp(['Real act: ' act]);
           %disp(num2str(predicted_class));
        end

        %disp("flagaa");
    catch ME
        disp(['Lỗi khi xử lý tin nhắn: ' ME.message]);
    end
end
