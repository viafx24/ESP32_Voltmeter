clear all;
close all;

Succeed=0;
Failed=0;
Iteration=0;
for i=1:1000
    Iteration=Iteration+1
    t = tcpclient("192.168.0.18",80,"Timeout",10)
    A=0;
    pause(5);
    
    while ~isempty(A)
        
        A=t.readline()
               
        if isempty(A) 
            pause(0.5);
            disp("error")
            Failed=Failed+1;
            
               
        else
            Succeed=Succeed+1
            Failed
            B{Iteration}=A
        end
        
    end
end