%% A MATLAB GUI to obtain voltage and current data sent by the ESP32.
%Data are received as a string line with different value separated by a
%comma.
% the function TCPCLIENT() establish the connexion with the ESP32 and
% receive data. Then the line is split and convert in number to feed the
% preallocated Data matrix. 

% the functionning of MATLAB GUI using the old system "uicontrol" is mainly
% the following:
% (1)most uicontrol and all data that may be used in callback should be put
% inside a structure called handles.
% (2)Then, those handles have to be put in the function guidata to actualize
% their presence (in the exemple above, handles.p2 represent the parent:
% here a panel. Command: guidata(handles.p2,handles);
% (3) to retrieve the data for instance, in a callback, we use the following
% code: handles=guidata(source); by this way, all the data in the structure
% handles can get access and modify.
%(4) Each time modified, there is still a need to call the command: 
% guidata(handles.p2,handles); Once those 4 points understood, GUI using
% uicontrol are easier to understand.

close all;
clear all;


%% GUI preparation

f = figure('Visible','on', 'units', 'normalized');%,'Position',[360,500,450,285]


p = uipanel('Title','Control','FontSize',12,...
    'BackgroundColor','white',...
    'Position',[.02 .02 .20 0.98]);

handles.p2 = uipanel('Title','Data','FontSize',12,...
    'BackgroundColor','white',...
    'Position',[.23 .02 .76 0.98]);


tg = uitabgroup(handles.p2);
handles.tab1 = uitab(tg,'Title','Dynamic');
handles.tab2 = uitab(tg,'Title','Static');



sp1 = uipanel('Parent',p,'Title','ADS1115','FontSize',12,...
    'Position',[0 .40 1 .40]);
sp2 = uipanel('Parent',p,'Title','ESP32','FontSize',12,...
    'Position',[0 .15 1 .25]);

sp3 = uipanel('Parent',p,'Title','Main','FontSize',12,...
    'Position',[0 0.80 1 .2]);

sp4 = uipanel('Parent',p,'Title','Info','FontSize',12,...
    'Position',[0 0 1 0.15]);


handles.hrun = uicontrol('Parent',sp3,'String','Run',...
    'Units', 'Normalized','Position',[0.02 0.51 0.95 0.47],'Callback',@hrun_Callback);

handles.hstop = uicontrol('Parent',sp3,'String','Stop',...
    'Units', 'Normalized','Position',[0.02 0.02 0.95 0.47],'Callback',@hstop_Callback);


handles.hCheckBox_A0    = uicontrol('Parent',sp1,'Style','checkbox',...
    'String','A0', 'Units', 'Normalized','Position',[0.01 0.88 0.95 0.1],...
    'Callback',@hCheckBox_Callback_A0);

handles.hCheckBox_A1    = uicontrol('Parent',sp1,'Style','checkbox',...
    'String','A1', 'Units', 'Normalized','Position',[0.01 0.76 0.95 0.1],...
    'Callback',@hCheckBox_Callback_A1);

handles.hCheckBox_A2    = uicontrol('Parent',sp1,'Style','checkbox',...
    'String','A2', 'Units', 'Normalized','Position',[0.01 0.64 0.95 0.1],...
    'Callback',@hCheckBox_Callback_A2);

handles.hCheckBox_A3    = uicontrol('Parent',sp1,'Style','checkbox',...
    'String','A3', 'Units', 'Normalized','Position',[0.01 0.52 0.95 0.1],...
    'Callback',@hCheckBox_Callback_A3);

handles.hCheckBox_A0_A1V    = uicontrol('Parent',sp1,'Style','checkbox',...
    'String','A0-A1(V)', 'Units', 'Normalized','Position',[0.01 0.40 0.95 0.1],...
    'Callback',@hCheckBox_Callback_A0_A1V);

handles.hCheckBox_A0_A1mA    = uicontrol('Parent',sp1,'Style','checkbox',...
    'String','A0-A1(mA)', 'Units', 'Normalized','Position',[0.01 0.28 0.95 0.1],...
    'Callback',@hCheckBox_Callback_A0_A1mA);

handles.hCheckBox_A2_A3V  = uicontrol('Parent',sp1,'Style','checkbox',...
    'String','A2-A3(V)', 'Units', 'Normalized','Position',[0.01 0.16 0.95 0.1],...
    'Callback',@hCheckBox_Callback_A2_A3V);

handles.hCheckBox_A2_A3mA    = uicontrol('Parent',sp1,'Style','checkbox',...
    'String','A2-A3(mA)', 'Units', 'Normalized','Position',[0.01 0.04 0.95 0.1],...
    'Callback',@hCheckBox_Callback_A2_A3mA);

handles.hCheckBox_34    = uicontrol('Parent',sp2,'Style','checkbox',...
    'String','34', 'Units', 'Normalized','Position',[0.01 0.70 0.95 0.2],...
    'Callback',@hCheckBox_Callback_34);

handles.hCheckBox_35    = uicontrol('Parent',sp2,'Style','checkbox',...
    'String','35', 'Units', 'Normalized','Position',[0.01 0.50 0.95 0.2],...
    'Callback',@hCheckBox_Callback_35);

handles.hCheckBox_32    = uicontrol('Parent',sp2,'Style','checkbox',...
    'String','32', 'Units', 'Normalized','Position',[0.01 0.30 0.95 0.2],...
    'Callback',@hCheckBox_Callback_32);

handles.hCheckBox_33    = uicontrol('Parent',sp2,'Style','checkbox',...
    'String','33', 'Units', 'Normalized','Position',[0.01 0.10 0.95 0.2],...
    'Callback',@hCheckBox_Callback_33);


handles.label1    = uicontrol('Parent',sp4,'Style','text',...
    'String','Program number :','HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.01 0.55 1 0.45]);


handles.label2    = uicontrol('Parent',sp4,'Style','text',...
    'String','','FontSize',14, 'Units', 'Normalized','Position',[0.01 0.1 1 0.54]);


Label_FontSize_1=20;
Label_FontSize_2=16;

handles.label_A0=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','A0:','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.02 0.80 0.2 0.1]);

handles.label_A0_Value=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.18 0.80 0.33 0.1]);

handles.label_A1=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','A1:','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.02 0.65 0.2 0.1]);

handles.label_A1_Value=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.18 0.65 0.33 0.1]);

handles.label_A2=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','A2:','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.02 0.5 0.2 0.1]);

handles.label_A2_Value=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.18 0.5 0.33 0.1]);

handles.label_A3=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','A3:','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.02 0.35 0.2 0.1]);

handles.label_A3_Value=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.18 0.35 0.33 0.1]);

handles.label_A0_A1V=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','A0-A1:','FontSize',Label_FontSize_2,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.02 0.2 0.2 0.1]);

handles.label_A0_A1V_Value=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','','FontSize',Label_FontSize_2,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.24 0.2 0.33 0.1]);

handles.label_A2_A3V=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','A2-A3:','FontSize',Label_FontSize_2,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.02 0.05 0.2 0.1]);

handles.label_A2_A3V_Value=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','','FontSize',Label_FontSize_2,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.24 0.05 0.33 0.1]);

handles.label_34=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','34:','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.5 0.8 0.2 0.1]);

handles.label_34_Value=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.66 0.8 0.33 0.1]);


handles.label_35=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','35:','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.5 0.65 0.2 0.1]);

handles.label_35_Value=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.66 0.65 0.33 0.1]);


handles.label_32=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','32:','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.5 0.5 0.2 0.1]);

handles.label_32_Value=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.66 0.5 0.33 0.1]);

handles.label_33=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','33:','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.5 0.35 0.2 0.1]);

handles.label_33_Value=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','','FontSize',Label_FontSize_1,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.66 0.35 0.33 0.1]);

handles.label_A0_A1mA=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','A0-A1(mA):','FontSize',Label_FontSize_2,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.5 0.2 0.26 0.1]);

handles.label_A0_A1mA_Value=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','','FontSize',Label_FontSize_2,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.75 0.2 0.33 0.1]);

handles.label_A2_A3mA=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','A2-A3(mA):','FontSize',Label_FontSize_2,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.5 0.05 0.26 0.1]);

handles.label_A2_A3mA_Value=uicontrol('Parent',handles.tab2,'Style','text',...
    'String','','FontSize',Label_FontSize_2,'HorizontalAlignment','left', 'Units', 'Normalized','Position',[0.75 0.05 0.33 0.1]);


handles.hCheckBox_A0.Enable='off';
handles.hCheckBox_A1.Enable='off';
handles.hCheckBox_A2.Enable='off';
handles.hCheckBox_A3.Enable='off';
handles.hCheckBox_34.Enable='off';
handles.hCheckBox_35.Enable='off';
handles.hCheckBox_32.Enable='off';
handles.hCheckBox_33.Enable='off';
handles.hCheckBox_A0_A1V.Enable='off';
handles.hCheckBox_A0_A1mA.Enable='off';
handles.hCheckBox_A2_A3V.Enable='off';
handles.hCheckBox_A2_A3mA.Enable='off';

set(handles.hstop,'Enable','off');
handles.STOP=0;
guidata(handles.p2,handles);


%% Main callback

function hrun_Callback(source,eventdata,handles)

handles=guidata(source);

handles.tcp_obj = tcpclient("192.168.0.18",80,"Timeout",10);

pause(1);

Line=handles.tcp_obj.readline();
SplitLine=split(Line,",");
handles.Number_Program = str2double(SplitLine(1));

set(handles.label2,'String',num2str(handles.Number_Program));

Pause_Second = 0;
Number_Iteration =10000000;
precision=4;
Data=zeros(Number_Iteration, length(SplitLine));


switch handles.Number_Program
    case 0
        
        handles.hCheckBox_A0.Enable='on';
        
        t=tiledlayout(handles.tab1, 1,1);
        nexttile(t);
        handles.h_A0=plot(NaN,NaN);
        handles.hCheckBox_A0.Value=1;
        ylabel('Voltage');
        xlabel('Time');
        legend('A0');
        
    case 1
        
        handles.hCheckBox_A0.Enable='on';
        handles.hCheckBox_A1.Enable='on';
        
        t=tiledlayout(handles.tab1, 1,1);
        nexttile(t);
        hold on;
        handles.h_A0=plot(NaN,NaN);
        handles.h_A1=plot(NaN,NaN);
        handles.hCheckBox_A0.Value=1;
        handles.hCheckBox_A1.Value=1;
        ylabel('Voltage');
        xlabel('Time');
        legend('A0','A1');
        
        
    case 2
        
        handles.hCheckBox_A0.Enable='on';
        handles.hCheckBox_A1.Enable='on';
        handles.hCheckBox_A2.Enable='on';
        
        t=tiledlayout(handles.tab1, 1,1);
        nexttile(t);
        hold on;
        handles.h_A0=plot(NaN,NaN);
        handles.h_A1=plot(NaN,NaN);
        handles.h_A2=plot(NaN,NaN);
        handles.hCheckBox_A0.Value=1;
        handles.hCheckBox_A1.Value=1;
        handles.hCheckBox_A2.Value=1;
        ylabel('Voltage');
        xlabel('Time');
        legend('A0','A1','A2');
        

    case 3
        
        handles.hCheckBox_A0.Enable='on';
        handles.hCheckBox_A1.Enable='on';
        handles.hCheckBox_A2.Enable='on';
        handles.hCheckBox_A3.Enable='on';
        
        
        t=tiledlayout(handles.tab1, 1,1);
        nexttile(t);
        hold on;
        handles.h_A0=plot(NaN,NaN);
        handles.h_A1=plot(NaN,NaN);
        handles.h_A2=plot(NaN,NaN);
        handles.h_A3=plot(NaN,NaN);
        handles.hCheckBox_A0.Value=1;
        handles.hCheckBox_A1.Value=1;
        handles.hCheckBox_A2.Value=1;
        handles.hCheckBox_A3.Value=1;
        ylabel('Voltage');
        xlabel('Time');
        legend('A0','A1','A2','A3');
        
        
    case 4
        
        handles.hCheckBox_A0.Enable='on';
        handles.hCheckBox_A1.Enable='on';
        handles.hCheckBox_A2.Enable='on';
        handles.hCheckBox_A3.Enable='on';
        handles.hCheckBox_34.Enable='on';
        
        t=tiledlayout(handles.tab1, 1,1);
        nexttile(t);
        hold on;
        handles.h_A0=plot(NaN,NaN);
        handles.h_A1=plot(NaN,NaN);
        handles.h_A2=plot(NaN,NaN);
        handles.h_A3=plot(NaN,NaN);
        handles.h_34=plot(NaN,NaN);
        handles.hCheckBox_A0.Value=1;
        handles.hCheckBox_A1.Value=1;
        handles.hCheckBox_A2.Value=1;
        handles.hCheckBox_A3.Value=1;
        handles.hCheckBox_34.Value=1;
        ylabel('Voltage');
        xlabel('Time');
        legend('A0','A1','A2','A3','34');
        
              
    case 5
        
        handles.hCheckBox_A0.Enable='on';
        handles.hCheckBox_A1.Enable='on';
        handles.hCheckBox_A2.Enable='on';
        handles.hCheckBox_A3.Enable='on';
        handles.hCheckBox_34.Enable='on';
        handles.hCheckBox_35.Enable='on';
        
        
        t=tiledlayout(handles.tab1, 1,1);
        nexttile(t);
        hold on;
        handles.h_A0=plot(NaN,NaN);
        handles.h_A1=plot(NaN,NaN);
        handles.h_A2=plot(NaN,NaN);
        handles.h_A3=plot(NaN,NaN);
        handles.h_34=plot(NaN,NaN);
        handles.h_35=plot(NaN,NaN);
        handles.hCheckBox_A0.Value=1;
        handles.hCheckBox_A1.Value=1;
        handles.hCheckBox_A2.Value=1;
        handles.hCheckBox_A3.Value=1;
        handles.hCheckBox_34.Value=1;
        handles.hCheckBox_35.Value=1;
        ylabel('Voltage');
        xlabel('Time');
        legend('A0','A1','A2','A3','34','35');
        
     
    case 6
        
        handles.hCheckBox_A0.Enable='on';
        handles.hCheckBox_A1.Enable='on';
        handles.hCheckBox_A2.Enable='on';
        handles.hCheckBox_A3.Enable='on';
        handles.hCheckBox_34.Enable='on';
        handles.hCheckBox_35.Enable='on';
        handles.hCheckBox_32.Enable='on';
        
        
        t=tiledlayout(handles.tab1, 1,1);
        nexttile(t);
        hold on;
        handles.h_A0=plot(NaN,NaN);
        handles.h_A1=plot(NaN,NaN);
        handles.h_A2=plot(NaN,NaN);
        handles.h_A3=plot(NaN,NaN);
        handles.h_34=plot(NaN,NaN);
        handles.h_35=plot(NaN,NaN);
        handles.h_32=plot(NaN,NaN);
        handles.hCheckBox_A0.Value=1;
        handles.hCheckBox_A1.Value=1;
        handles.hCheckBox_A2.Value=1;
        handles.hCheckBox_A3.Value=1;
        handles.hCheckBox_34.Value=1;
        handles.hCheckBox_35.Value=1;
        handles.hCheckBox_32.Value=1;
        ylabel('Voltage');
        xlabel('Time');
        legend('A0','A1','A2','A3','34','35','32');
        
      
    case 7
        
        handles.hCheckBox_A0.Enable='on';
        handles.hCheckBox_A1.Enable='on';
        handles.hCheckBox_A2.Enable='on';
        handles.hCheckBox_A3.Enable='on';
        handles.hCheckBox_34.Enable='on';
        handles.hCheckBox_35.Enable='on';
        handles.hCheckBox_32.Enable='on';
        handles.hCheckBox_33.Enable='on';
        
        t=tiledlayout(handles.tab1, 1,1);
        nexttile(t);
        hold on;
        handles.h_A0=plot(NaN,NaN);
        handles.h_A1=plot(NaN,NaN);
        handles.h_A2=plot(NaN,NaN);
        handles.h_A3=plot(NaN,NaN);
        handles.h_34=plot(NaN,NaN);
        handles.h_35=plot(NaN,NaN);
        handles.h_32=plot(NaN,NaN);
        handles.h_33=plot(NaN,NaN);
        handles.hCheckBox_A0.Value=1;
        handles.hCheckBox_A1.Value=1;
        handles.hCheckBox_A2.Value=1;
        handles.hCheckBox_A3.Value=1;
        handles.hCheckBox_34.Value=1;
        handles.hCheckBox_35.Value=1;
        handles.hCheckBox_32.Value=1;
        handles.hCheckBox_33.Value=1;
        ylabel('Voltage');
        xlabel('Time');
        legend('A0','A1','A2','A3','34','35','32','33');
        
             
    case 8
        
        handles.hCheckBox_A0_A1mA.Enable='on';
        handles.hCheckBox_A0_A1V.Enable='on';
        handles.hCheckBox_A2_A3mA.Enable='on';
        handles.hCheckBox_A2_A3V.Enable='on';
        handles.hCheckBox_34.Enable='on';
        handles.hCheckBox_35.Enable='on';
        handles.hCheckBox_32.Enable='on';
        handles.hCheckBox_33.Enable='on';
        
        
        t=tiledlayout(handles.tab1, 2,1);
        nexttile(t);
        hold on;
        
        handles.h_A0_A1V=plot(NaN,NaN);
        handles.h_A2_A3V=plot(NaN,NaN);
        
        
        handles.h_34=plot(NaN,NaN);
        handles.h_35=plot(NaN,NaN);
        handles.h_32=plot(NaN,NaN);
        handles.h_33=plot(NaN,NaN);
        
        
        
        ylabel('Voltage');
        xlabel('Time');
        legend('A0-A1','A2-A3','34','35','32','33');
        
        nexttile(t);
        hold on
        handles.h_A0_A1mA=plot(NaN,NaN);
        handles.h_A2_A3mA=plot(NaN,NaN);
        
        ylabel('Current');
        xlabel('Time');
        legend('A0-A1','A2-A3');
        
        handles.hCheckBox_A0_A1mA.Value=1;
        handles.hCheckBox_A0_A1V.Value=1;
        handles.hCheckBox_A2_A3mA.Value=1;
        handles.hCheckBox_A2_A3V.Value=1;
        handles.hCheckBox_34.Value=1;
        handles.hCheckBox_35.Value=1;
        handles.hCheckBox_32.Value=1;
        handles.hCheckBox_33.Value=1;
        
        
        
    case 9
        
        
        handles.hCheckBox_A0_A1mA.Enable='on';
        handles.hCheckBox_A0_A1V.Enable='on';
        handles.hCheckBox_A2_A3mA.Enable='on';
        handles.hCheckBox_A2_A3V.Enable='on';
        
        t=tiledlayout(handles.tab1, 2,1);
        nexttile(t);
        hold on;
        handles.h_A0_A1V=plot(NaN,NaN);
        handles.h_A2_A3V=plot(NaN,NaN);
        
        ylabel('Voltage');
        xlabel('Time');
        legend('A0-A1','A2-A3');
        
        nexttile(t);
        hold on;
        handles.h_A0_A1mA=plot(NaN,NaN);
        handles.h_A2_A3mA=plot(NaN,NaN);
        
        ylabel('Current');
        xlabel('Time');
        legend('A0-A1','A2-A3');
        
        handles.hCheckBox_A0_A1mA.Value=1;
        handles.hCheckBox_A0_A1V.Value=1;
        handles.hCheckBox_A2_A3mA.Value=1;
        handles.hCheckBox_A2_A3V.Value=1;
        
    case 10
        
        t=tiledlayout(handles.tab1, 2,1);
        nexttile(t);
        hold on;
        handles.h_A0_A1V=plot(NaN,NaN);
        handles.h_A2=plot(NaN,NaN);
        handles.h_A3=plot(NaN,NaN);
        
        ylabel('Voltage');
        xlabel('Time');
        legend('A0-A1','A2','A3');
        
        nexttile(t);
        hold on;
        handles.h_A0_A1mA=plot(NaN,NaN);
        
        ylabel('Current');
        xlabel('Time');
        legend('A0-A1');
        
        handles.hCheckBox_A0_A1mA.Enable='on';
        handles.hCheckBox_A0_A1V.Enable='on';
        handles.hCheckBox_A2.Enable='on';
        handles.hCheckBox_A3.Enable='on';
        
        
        handles.hCheckBox_A0_A1mA.Value=1;
        handles.hCheckBox_A0_A1V.Value=1;
        handles.hCheckBox_A2.Value=1;
        handles.hCheckBox_A3.Value=1;
        
        
        
    case 11
        
        t=tiledlayout(handles.tab1, 2,1);
        nexttile(t);
        hold on;
        handles.h_A0_A1V=plot(NaN,NaN);
        
        ylabel('Voltage');
        xlabel('Time');
        legend('A0-A1');
        
        nexttile(t);
        hold on;
        handles.h_A0_A1mA=plot(NaN,NaN);
        
        ylabel('Current');
        xlabel('Time');
        legend('A0-A1');
        
        handles.hCheckBox_A0_A1mA.Enable='on';
        handles.hCheckBox_A0_A1V.Enable='on';
        
        handles.hCheckBox_A0_A1mA.Value=1;
        handles.hCheckBox_A0_A1V.Value=1;
        
        
        
    case 12
        
        t=tiledlayout(handles.tab1, 1,1);
        nexttile(t);
        handles.h_A2_A3V=plot(NaN,NaN);
        
        ylabel('Voltage');
        xlabel('Time');
        legend('A2-A3');
        
        handles.hCheckBox_A2_A3V.Enable='on';
        handles.hCheckBox_A2_A3V.Value=1;
        
        
end

guidata(handles.p2,handles);

set(handles.hrun,'Enable','off');
set(handles.hstop,'Enable','on');

Wifi_Errors=0;

for i=1:Number_Iteration
    
    handles=guidata(source);
    guidata(handles.p2,handles);
    
    Line=handles.tcp_obj.readline(); % main line to get the data from the ESP32
    if  ~isempty(Line)
        SplitLine=split(Line,",");
        Data(i,:)=str2double(SplitLine)';% Split and convert in number
        
        if handles.hCheckBox_A0.Value == 1
            
            handles.h_A0.XData=Data(1:i,2)/1000;%millisecond to second
            handles.h_A0.YData=Data(1:i,3);
            handles.label_A0_Value.String=num2str(Data(i,3),precision);
            
        end
        
        if handles.hCheckBox_A1.Value == 1
            handles.h_A1.XData=Data(1:i,2)/1000;
            handles.h_A1.YData=Data(1:i,4);
            handles.label_A1_Value.String=num2str(Data(i,4),precision);
        end
        
        
        if handles.hCheckBox_A2.Value == 1
            
            handles.h_A2.XData=Data(1:i,2)/1000;
            handles.h_A2.YData=Data(1:i,5);
            handles.label_A2_Value.String=num2str(Data(i,5),precision);
        end
        
        
        if handles.hCheckBox_A3.Value == 1
            
            handles.h_A3.XData=Data(1:i,2)/1000;
            handles.h_A3.YData=Data(1:i,6);
            handles.label_A3_Value.String=num2str(Data(i,6),precision);
            
        end
        
        
        if handles.hCheckBox_34.Value == 1
            handles.h_34.XData=Data(1:i,2)/1000;
            handles.h_34.YData=Data(1:i,7);
            handles.label_34_Value.String=num2str(Data(i,7),precision);
            
        end
        
        if handles.hCheckBox_35.Value == 1
            handles.h_35.XData=Data(1:i,2)/1000;
            handles.h_35.YData=Data(1:i,8);
            handles.label_35_Value.String=num2str(Data(i,8),precision);
        end
        
        if handles.hCheckBox_32.Value == 1
            handles.h_32.XData=Data(1:i,2)/1000;
            handles.h_32.YData=Data(1:i,9);
            handles.label_32_Value.String=num2str(Data(i,9),precision);
        end
        
        if handles.hCheckBox_33.Value == 1
            handles.h_33.XData=Data(1:i,2)/1000;
            handles.h_33.YData=Data(1:i,10);
            handles.label_33_Value.String=num2str(Data(i,10),precision);
        end
        
        
        if handles.hCheckBox_A0_A1V.Value == 1
            handles.h_A0_A1V.XData=Data(1:i,2)/1000;
            handles.h_A0_A1V.YData=Data(1:i,4);
            handles.label_A0_A1V_Value.String=num2str(Data(i,4),precision);
        end
        
        if handles.hCheckBox_A2_A3V.Value == 1
            handles.h_A2_A3V.XData=Data(1:i,2)/1000;
            if handles.Number_Program == 12
                handles.h_A2_A3V.YData=Data(1:i,3);
                handles.label_A2_A3V_Value.String=num2str(Data(i,3),precision);
            else
                handles.h_A2_A3V.YData=Data(1:i,6);
                handles.label_A2_A3V_Value.String=num2str(Data(i,6),precision);
            end
        end
        
        if handles.hCheckBox_A0_A1mA.Value == 1
            handles.h_A0_A1mA.XData=Data(1:i,2)/1000;
            handles.h_A0_A1mA.YData=Data(1:i,3);
            handles.label_A0_A1mA_Value.String=num2str(Data(i,3),precision);
        end
        
        if handles.hCheckBox_A2_A3mA.Value == 1
            handles.h_A2_A3mA.XData=Data(1:i,2)/1000;
            handles.h_A2_A3mA.YData=Data(1:i,5);
            handles.label_A2_A3mA_Value.String=num2str(Data(i,5),precision);
        end
                
        drawnow;
        pause(Pause_Second);
        
        
        
        if handles.STOP == 1
            
            %constantly erase backup thus save interesting data before new
            %run
            Data_Backup_Volmeter=Data(1:i,:);
            save('Temp_Back_Up_Volmeter.mat','Data_Backup_Volmeter');
            set(handles.hrun,'Enable','on');
            set(handles.hstop,'Enable','off');
            
            handles.STOP=0;
                        
            handles.hCheckBox_A0.Value=0;
            handles.hCheckBox_A1.Value=0;
            handles.hCheckBox_A2.Value=0;
            handles.hCheckBox_A3.Value=0;
            handles.hCheckBox_34.Value=0;
            handles.hCheckBox_35.Value=0;
            handles.hCheckBox_32.Value=0;
            handles.hCheckBox_33.Value=0;
            handles.hCheckBox_A0_A1mA.Value=0;
            handles.hCheckBox_A0_A1V.Value=0;
            handles.hCheckBox_A2_A3mA.Value=0;
            handles.hCheckBox_A2_A3V.Value=0;
            
            
            handles.hCheckBox_A0.Enable='off';
            handles.hCheckBox_A1.Enable='off';
            handles.hCheckBox_A2.Enable='off';
            handles.hCheckBox_A3.Enable='off';
            handles.hCheckBox_34.Enable='off';
            handles.hCheckBox_35.Enable='off';
            handles.hCheckBox_32.Enable='off';
            handles.hCheckBox_33.Enable='off';
            handles.hCheckBox_A0_A1mA.Enable='off';
            handles.hCheckBox_A0_A1V.Enable='off';
            handles.hCheckBox_A2_A3mA.Enable='off';
            handles.hCheckBox_A2_A3V.Enable='off';
            
            
            switch handles.Number_Program
                
                case 0
                    
                    delete(handles.h_A0)      ;
                    handles.label_A0_Value.String='';
                    
                case 1
                    
                    delete(handles.h_A0)      ;
                    handles.label_A0_Value.String='';
                    delete(handles.h_A1);
                    handles.label_A1_Value.String='';
                    
                case 2
                    
                    delete(handles.h_A0)     ;
                    handles.label_A0_Value.String='';
                    delete(handles.h_A1);
                    handles.label_A1_Value.String='';
                    delete(handles.h_A2);
                    handles.label_A2_Value.String='';
                    
                case 3
                    
                    delete(handles.h_A0);
                    handles.label_A0_Value.String='';
                    delete(handles.h_A1);
                    handles.label_A1_Value.String='';
                    delete(handles.h_A2);
                    handles.label_A2_Value.String='';
                    delete(handles.h_A3);
                    handles.label_A3_Value.String='';
                    
                case 4
                    
                    delete(handles.h_A0)      ;
                    handles.label_A0_Value.String='';
                    delete(handles.h_A1);
                    handles.label_A1_Value.String='';
                    delete(handles.h_A2);
                    handles.label_A2_Value.String='';
                    delete(handles.h_A3);
                    handles.label_A3_Value.String='';
                    delete(handles.h_34);
                    handles.label_34_Value.String='';
                    
                case 5
                    
                    delete(handles.h_A0)     ;
                    handles.label_A0_Value.String='';
                    delete(handles.h_A1);
                    handles.label_A1_Value.String='';
                    delete(handles.h_A2);
                    handles.label_A2_Value.String='';
                    delete(handles.h_A3);
                    handles.label_A3_Value.String='';
                    delete(handles.h_34);
                    handles.label_34_Value.String='';
                    delete(handles.h_35);
                    handles.label_35_Value.String='';
                    
                case 6
                    
                    delete(handles.h_A0)     ;
                    handles.label_A0_Value.String='';
                    delete(handles.h_A1);
                    handles.label_A1_Value.String='';
                    delete(handles.h_A2);
                    handles.label_A2_Value.String='';
                    delete(handles.h_A3);
                    handles.label_A3_Value.String='';
                    delete(handles.h_34);
                    handles.label_34_Value.String='';
                    delete(handles.h_35);
                    handles.label_35_Value.String='';
                    delete(handles.h_32);
                    handles.label_32_Value.String='';
                    
                case 7
                    
                    delete(handles.h_A0) ;
                    handles.label_A0_Value.String='';
                    delete(handles.h_A1);
                    handles.label_A1_Value.String='';
                    delete(handles.h_A2);
                    handles.label_A2_Value.String='';
                    delete(handles.h_A3);
                    handles.label_A3_Value.String='';
                    delete(handles.h_34);
                    handles.label_34_Value.String='';
                    delete(handles.h_35);
                    handles.label_35_Value.String='';
                    delete(handles.h_32);
                    handles.label_32_Value.String='';
                    delete(handles.h_33);
                    handles.label_33_Value.String='';
                    
                case 8
                    
                    delete(handles.h_A0_A1mA);
                    handles.label_A0_A1mA_Value.String='';
                    delete(handles.h_A0_A1V);
                    handles.label_A0_A1V_Value.String='';
                    delete(handles.h_A2_A3mA);
                    handles.label_A2_A3mA_Value.String='';
                    delete(handles.h_A2_A3V);
                    handles.label_A2_A3V_Value.String='';
                    delete(handles.h_34);
                    handles.label_34_Value.String='';
                    delete(handles.h_35);
                    handles.label_35_Value.String='';
                    delete(handles.h_32);
                    handles.label_32_Value.String='';
                    delete(handles.h_33);
                    handles.label_33_Value.String='';
                    
                case 9
                    
                    delete(handles.h_A0_A1mA);
                    handles.label_A0_A1mA_Value.String='';
                    delete(handles.h_A0_A1V);
                    handles.label_A0_A1V_Value.String='';
                    delete(handles.h_A2_A3mA);
                    handles.label_A2_A3mA_Value.String='';
                    delete(handles.h_A2_A3V);
                    handles.label_A2_A3V_Value.String='';
                    
                case 10
                    
                    delete(handles.h_A0_A1mA);
                    handles.label_A0_A1mA_Value.String='';
                    delete(handles.h_A0_A1V);
                    handles.label_A0_A1V_Value.String='';
                    delete(handles.h_A2);
                    handles.label_A2_Value.String='';
                    delete(handles.h_A3);
                    handles.label_A3_Value.String='';
                    
                case 11
                    
                    delete(handles.h_A0_A1mA);
                    handles.label_A0_A1mA_Value.String='';
                    delete(handles.h_A0_A1V);
                    handles.label_A0_A1V_Value.String='';
                    
                case 12
                    
                    delete(handles.h_A2_A3V);
                    handles.label_A2_A3V_Value.String='';
                    
            end
            
            delete(handles.tcp_obj);
            %delete(handles.Number_Program);% doesn't not work, don't know
            %why.
            set(handles.label2,'String','');
            guidata(handles.p2,handles);
            break;
        end
    else
        
%      Wifi_Errors=  Wifi_Errors + 1;
%      pause(1);
%      disp(['Number of Wifi Errors:' num2str(Wifi_Errors)]);
        
    end
end

end


%% Other callbacks

function hstop_Callback(source,eventdata,handles)

handles=guidata(source);

handles.STOP=1;

guidata(handles.p2,handles);


end


function hCheckBox_Callback_A0(source,eventdata,handles)

handles=guidata(source);

Value = get( handles.hCheckBox_A0, 'Value');

if Value==0
    set(handles.h_A0,'Visible','off');
else
    set(handles.h_A0,'Visible','on');
end

end

function hCheckBox_Callback_A1(source,eventdata,handles)

handles=guidata(source);

Value = get( handles.hCheckBox_A1, 'Value');

if Value==0
    set(handles.h_A1,'Visible','off');
else
    set(handles.h_A1,'Visible','on');
end

end

function hCheckBox_Callback_A2(source,eventdata,handles)

handles=guidata(source);

Value = get( handles.hCheckBox_A2, 'Value');

if Value==0
    set(handles.h_A2,'Visible','off');
else
    set(handles.h_A2,'Visible','on');
end

end


function hCheckBox_Callback_A3(source,eventdata,handles)

handles=guidata(source);

Value = get( handles.hCheckBox_A3, 'Value');

if Value==0
    set(handles.h_A3,'Visible','off');
else
    set(handles.h_A3,'Visible','on');
end

end



function hCheckBox_Callback_34(source,eventdata,handles)

handles=guidata(source);

Value = get( handles.hCheckBox_34, 'Value');

if Value==0
    set(handles.h_34,'Visible','off');
else
    set(handles.h_34,'Visible','on');
end

end

function hCheckBox_Callback_35(source,eventdata,handles)

handles=guidata(source);

Value = get( handles.hCheckBox_35, 'Value');

if Value==0
    set(handles.h_35,'Visible','off');
else
    set(handles.h_35,'Visible','on');
end

end

function hCheckBox_Callback_33(source,eventdata,handles)

handles=guidata(source);

Value = get( handles.hCheckBox_33, 'Value');

if Value==0
    set(handles.h_33,'Visible','off');
else
    set(handles.h_33,'Visible','on');
end

end

function hCheckBox_Callback_32(source,eventdata,handles)

handles=guidata(source);

Value = get( handles.hCheckBox_32, 'Value');

if Value==0
    set(handles.h_32,'Visible','off');
else
    set(handles.h_32,'Visible','on');
end

end


function hCheckBox_Callback_A0_A1V(source,eventdata,handles)

handles=guidata(source);

Value = get( handles.hCheckBox_A0_A1V, 'Value');

if Value==0
    set(handles.h_A0_A1V,'Visible','off');
else
    set(handles.h_A0_A1V,'Visible','on');
end

end

function hCheckBox_Callback_A0_A1mA(source,eventdata,handles)

handles=guidata(source);

Value = get( handles.hCheckBox_A0_A1mA, 'Value');

if Value==0
    set(handles.h_A0_A1mA,'Visible','off');
else
    set(handles.h_A0_A1mA,'Visible','on');
end

end


function hCheckBox_Callback_A2_A3V(source,eventdata,handles)

handles=guidata(source);

Value = get( handles.hCheckBox_A2_A3V, 'Value');

if Value==0
    set(handles.h_A2_A3V,'Visible','off');
else
    set(handles.h_A2_A3V,'Visible','on');
end

end

function hCheckBox_Callback_A2_A3mA(source,eventdata,handles)

handles=guidata(source);

Value = get( handles.hCheckBox_A2_A3mA, 'Value');

if Value==0
    set(handles.h_A2_A3mA,'Visible','off');
else
    set(handles.h_A2_A3mA,'Visible','on');
end

end