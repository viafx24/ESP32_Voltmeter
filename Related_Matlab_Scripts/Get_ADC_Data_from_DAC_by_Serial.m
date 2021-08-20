instrreset % maybe my friend

clear all;
close all;

if isequal(exist('s','var'),0)% ouvre serial si pas encore fait.
    s=serial('COM6','BaudRate',115200);
end

fopen(s);


Length_Data=60000;

Delay_Between_Acquisition=1; % seconds

[Data.Iteration,Data.ADC_ADS1115,Data.ADC_ADS1115_Voltage,Data.ADC_GPIO34,Data.ADC_GPIO34_Voltage]=deal(zeros(1,Length_Data));


f=figure;

tiledlayout(2,1);
ax1=nexttile;
hold on;
h1=plot(NaN,NaN,'-+b');
h2=plot(NaN,NaN,'-+r');
ylabel('ADC')
xlabel('Iteration DAC')
%ylim([0 5])
ax2=nexttile;
hold on;
h3=plot(NaN,NaN,'-+b');
h4=plot(NaN,NaN,'-+r');
ylabel('Voltage')
xlabel('Iteration DAC')
%ylim([0 500])


flushoutput(s)
flushinput(s)
pause(2)

for i=1:Length_Data
    %    tic;
    
%     flushoutput(s);
%     flushinput(s);
    
    RetrieveData=fscanf(s);
    
    commas = strfind(RetrieveData,',');
    while length(commas) ~= 4
        commas = strfind(RetrieveData,',');
    end
    if length(commas)==4
        
        Data.Iteration(i) = str2double(RetrieveData(1:commas(1)-1));      
        Data.ADC_ADS1115(i) = str2double(RetrieveData(commas(1):commas(2)-1));
        Data.ADC_ADS1115_Voltage(i) = str2double(RetrieveData(commas(2):commas(3)-1));
        Data.ADC_GPIO34(i) = str2double(RetrieveData(commas(3):commas(4)-1));
        Data.ADC_GPIO34_Voltage(i) = str2double(RetrieveData(commas(4):end));
         
    end
    

    h1.XData=1:i;
    h1.YData=Data.ADC_ADS1115(1:i);
    
    h2.XData=1:i;
    h2.YData=(Data.ADC_GPIO34(1:i) * 2^15 * 0.5079) / 2^12;
    
    h3.XData=1:i;
    h3.YData=Data.ADC_ADS1115_Voltage(1:i);
    
    h4.XData=1:i;
    h4.YData=Data.ADC_GPIO34_Voltage(1:i)
    
    drawnow;
%    pause(Delay_Between_Acquisition);
 

end