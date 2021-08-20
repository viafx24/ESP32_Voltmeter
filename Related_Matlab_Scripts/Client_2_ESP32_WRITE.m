% still some problems

clear all;
close all;

t = tcpclient("192.168.0.19",80)
Data='HIGH';
%write(t,Data);

for i=1:3600
    
    
    if mod(i,2)==0
        Data='HIGH';
    else
        Data='LOW';
    end    
        
   %writeline(t,Data);
   flush(t);
   write(t,Data);
    i
    pause(1);
end