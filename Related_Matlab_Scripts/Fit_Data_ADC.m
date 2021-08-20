
%% Prepare figure and load data

close all;
load('DataFit');
figure;
hold on;


%% fit a polynome on the ADC GPIO 34 data


y1=Data.ADC_GPIO34(120:363);
x1=1:length(y1);
FitObject_1 = fit(x1',y1','poly7');

MeasuredY=3000; ClosedX=100; 

f = @(x) polyval(coeffvalues(FitObject_1),x) - MeasuredY;

DesiredX = fzero(f,ClosedX);

plot(x1,y1,'+-b');
plot(FitObject_1,'r');

plot(DesiredX,MeasuredY,'og','MarkerSize',12)

%% fit a line on the ADC ads1115 data

y2_16bit=Data.ADC_ADS1115(120:363)  ;
y2_12bit=Data.ADC_ADS1115(120:363) *2^12 / 2^15  ;

y2=y2_12bit * 1.8618 %.* (y1/y2_12bit);


x2=1:length(y1);
FitObject_2 = fit(x2',y2','poly1');


plot(x2,y2,'+-b');
plot(FitObject_2,'r');

DesiredY = FitObject_2(DesiredX);

plot(DesiredX,DesiredY,'og','MarkerSize',12)
