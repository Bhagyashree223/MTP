function []=frame_selection()
filename='data_xy.txt';
smooth(filename);
fitBspline();


function []=fitBspline()
b=importdata('dumbell.txt');
a=b(:,1:2);
sz=size(a,1);
knots = linspace(0,1,sz+4);
points=0.95*[a(:,1)';a(:,2)'];
sp=spmak(knots,[points]);
figure;plot(a(:,1),a(:,2));
figure;fnplt(sp,knots([4 sz+1]));

dsp=fnder(sp,1);
%change 1 to 2 for second derivative


pt=fnval(dsp,knots(:,3:sz+2));
pt1=zeros(1,sz);
for i=1:sz
    pt1(1,i)=pt(2,i)/pt(1,i);
end


curr=1;
thresh=30;
cnt=2;
selected_frames=zeros(sz,1);
selected_frames(1)=b(1,3);
x1=zeros(sz,1);
y1=zeros(sz,1);
pt2=fnval(sp,knots(:,3:sz+2));
x1(1)=pt2(1,1);
y1(1)=pt2(2,1);

for i=2:sz
    if abs(pt1(1,i)-pt1(1,curr))>=thresh
        selected_frames(cnt)=b(i,3);
        x1(cnt)=pt2(1,i);
        y1(cnt)=pt2(2,i);
        cnt=cnt+1;
        curr=i;
    end
end

fp=fopen('./bsplineder.txt','wt');
for i=1:cnt
    fprintf(fp,'%d\n',selected_frames(i));
end


figure;plot(a(:,1),pt1');
figure;fnplt(sp,knots([4 sz+1]));
hold on; plot(x1,y1,'ro');



function []= smooth(filename)
b=importdata(filename);
a=b(:,1:2);
y=sgolayfilt(a,3,41);
fp=fopen('dumbell.txt','w');
plot(y(:,1),y(:,2));
for i=1:size(y,1)
    fprintf(fp,'%f %f %d\n',y(i,1),y(i,2),b(i,3));
end
fclose(fp);



