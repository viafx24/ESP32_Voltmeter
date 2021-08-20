close all;
%load(Data_Rampe_Before_Interp.mat);

Indices=find(Data.Iteration==1);
Begin=Indices(1); %1;
End=Indices(2)-5;%252;

Choice_Limit=4086;% interp cross the end with 16-1 value.

Data.ADC_ADS1115_0_Interp= uint16(interp(Data.ADC_ADS1115_0(Begin:End),16));
Data.ADC_GPIO34_Interp=uint16(interp(Data.ADC_GPIO34(Begin:End),16));
Data.Voltage_Bridge_ADS_Interp=interp(Data.Voltage_Bridge_ADS(Begin:End),16);


Data.ADC_GPIO34_Interp_Copie=Data.ADC_GPIO34_Interp;
Data.Voltage_Bridge_ADS_Interp_Copie=Data.Voltage_Bridge_ADS_Interp;

length(Data.ADC_GPIO34(Begin:End))
length(Data.ADC_GPIO34_Interp)
length(Data.Voltage_Bridge_ADS(Begin:End))
length(Data.Voltage_Bridge_ADS_Interp)

fileID = fopen('Data.txt','w');

formatSpec = '%d, %2.4f,\r\n';

for i=1: 4096 % length(Data.ADC_GPIO34_Interp)
    
    if Data.ADC_GPIO34_Interp(i)==0
        fprintf(fileID,formatSpec,Data.ADC_GPIO34_Interp(i),220);% aime of 220 is to signal a problem
        
        while(Data.ADC_GPIO34_Interp(i)==0)
            Data.ADC_GPIO34_Interp(1)=[];
            Data.Voltage_Bridge_ADS_Interp(1)=[];
        end
    end
    
    if  i <= Choice_Limit
        
        if Data.ADC_GPIO34_Interp(i)==i
            
            if i < (Choice_Limit-1) && (Data.ADC_GPIO34_Interp(i)>=Data.ADC_GPIO34_Interp(i+1)  )
                
                
                fprintf(fileID,formatSpec,Data.ADC_GPIO34_Interp(i),(Data.Voltage_Bridge_ADS_Interp(i)+ Data.Voltage_Bridge_ADS_Interp(i+1))/2);
                
                while(Data.ADC_GPIO34_Interp(i)  >=  Data.ADC_GPIO34_Interp(i+1))
                    Data.ADC_GPIO34_Interp(i+1)=[];
                    Data.Voltage_Bridge_ADS_Interp(i+1)=[];
                end
                
                
                
                
            else
                
                fprintf(fileID,formatSpec,Data.ADC_GPIO34_Interp(i),Data.Voltage_Bridge_ADS_Interp(i));
            end
            
            
        elseif  Data.ADC_GPIO34_Interp(i)>i
            
            fprintf(fileID,formatSpec,i,(Data.Voltage_Bridge_ADS_Interp(i-1) + Data.Voltage_Bridge_ADS_Interp(i))/2);
            Data.ADC_GPIO34_Interp=[Data.ADC_GPIO34_Interp(1:i-1) i Data.ADC_GPIO34_Interp(i:end)];
            Data.Voltage_Bridge_ADS_Interp=[Data.Voltage_Bridge_ADS_Interp(1:i-1) (Data.Voltage_Bridge_ADS_Interp(i-1) + Data.Voltage_Bridge_ADS_Interp(i))/2 ...
                Data.Voltage_Bridge_ADS_Interp(i:end)];
            
            
        end
    end
    
    if i > Choice_Limit
        
        fprintf(fileID,formatSpec,i,230);
        
    end
    
    
    
end

fclose(fileID);


figure
hold on

%plot(Data.ADC_GPIO34_Interp_Copie(1:4086),Data.Voltage_Bridge_ADS_Interp_Copie(1:4086),'-+r')
plot(Data.ADC_GPIO34_Interp_Copie,Data.Voltage_Bridge_ADS_Interp_Copie,'-db')
%plot(Data.ADC_GPIO34_Interp,Data.Voltage_Bridge_ADS_Interp,'-db')
plot(Data.ADC_GPIO34_Interp(1:4086),Data.Voltage_Bridge_ADS_Interp(1:4086),'-+g')
plot(Data.ADC_GPIO34(Begin:End),Data.Voltage_Bridge_ADS(Begin:End),'sr','MarkerSize',14);

