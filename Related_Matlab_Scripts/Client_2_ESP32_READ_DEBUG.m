clear all;
close all;

Succeed=0;
Failed=0;

t = tcpclient("192.168.0.19",80)
%flush(t);
for i=1:10000


   %A=t.readline()
   i
   A=t.readline()
 
 
end