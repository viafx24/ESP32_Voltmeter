clear all;
close all;

Succeed=0;
Failed=0;

t = tcpclient("192.168.0.18",80,"Timeout",10)

pause(2);
Number_Iteration=10000000;

Line=t.readline();
SplitLine=split(Line,",");

Data=zeros(Number_Iteration, length(SplitLine));

f=figure;
% tiledlayout(2,1);
% ax1=nexttile;
h1=plot(NaN,NaN,'-+b');
ylabel('Voltage')
xlabel('Time')
%ylim([0 5])
% ax2=nexttile;
% h2=plot(NaN,NaN,'-+r');
% ylabel('Current')
% xlabel('Time')
%ylim([0 500])

%set(p1,'Visible','off')

tic
for i = 1:Number_Iteration
    
   
    Line=t.readline();
    if  ~isempty(Line)
        SplitLine=split(Line,",");
        Data(i,:)=str2double(SplitLine)';
    
        h1.XData=Data(1:i,2)/1000;
        h1.YData=Data(1:i,3)*(98000 + 9870) / 9870;
        drawnow;  
    end
    
    
%pause(1);
%save('Discharging_Battery_Volmeter','Data');


end


toc



% for i=1:10000
%
%
%     A=0;

%     while ~isempty(A)
%         if isempty(A)
%             pause(0.5);
%             disp("error")
%             Failed=Failed+1;
%             B
%
%         else
%             B=A
%             Succeed=Succeed+1
%             Failed
%         end

%     end
%  toc
%     %     end

% end