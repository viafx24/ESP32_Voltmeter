clear all;
close all;

Succeed=0;
Failed=0;

t = tcpclient("192.168.0.18",80,"Timeout",10)
B=0;
pause(5);

for i=1:10000


A=0;

while ~isempty(A)
    
   A=t.readline()
   if isempty(A)
       pause(0.5);
       disp("error")
       Failed=Failed+1;
       B

   else
       B=A
       Succeed=Succeed+1
       Failed
   end
 
end

end