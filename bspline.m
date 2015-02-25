b=importdata('dumbell.txt');
a=b(:,1:2);
knots = linspace(0,1,354);
%pp1 = splinefit(a(:,1),a(:,2),20);
%pp1=fit(a(:,1),a(:,2),'cubicinterp');
% sp1 = fastBSpline.lsqspline(knots,3,a(:,1),a(:,2))
% figure;plot(a(:,1),a(:,2),'o',a(:,1),sp1.evalAt(a(:,1)));
%  
%  t = linspace( 0, 1, size(a,1) ); % define the parameter t
%  %size(t)
%  fitX = fit( t',a(:,1), 'cubicinterp'); % fit x as a function of parameter t
%  fitY = fit( t',a(:,2), 'cubicinterp'); % fit y as a function of parameter t
%  size(fitX)
%  
%  size(fitY)
% figure; plot( fitX ); % plot the parametric curve
points=0.95*[a(:,1)';a(:,2)'];
sp=spmak(knots,[points]);
figure;plot(a(:,1),a(:,2));
figure;fnplt(sp,[0,4]);

dsp=fnder(sp,1);
%change 1 to 2 for first derivative


%figure;fnplt(dsp,'j',[0,1]);
selected_points=zeros(350,1);
pt=fnval(dsp,a(:,1));
curr=1;
thresh=1.5;
cnt=1;

for i=2:350
    if abs(pt(1,i)-pt(1,curr))>=thresh
        selected_points(cnt)=a(i,1);
        cnt=cnt+1;
        curr=i;
    end
end
selected_points1=zeros(350,1);
curr1=1;
cnt1=1;
for i=2:350
    if abs(pt(2,i)-pt(2,curr1))>=thresh
        selected_points1(cnt1)=a(i,1);
        cnt1=cnt1+1;
        curr1=i;
    end
end
cnt1
selected_points=union(selected_points,selected_points1);
cnt=size(selected_points,1);
fp=fopen('bsplineder.txt','wt');
pt1=zeros(350,1);
x1=zeros(350,1);
cnt1=1;
for i=1:cnt
    for j=1:350
        if selected_points(i)==b(j,1)
             pt1(cnt1)=b(j,2);
             x1(cnt1)=b(j,1);
             cnt1=cnt1+1;
             fprintf(fp,'%d\n',b(j,3));
        end
    end
end
% pt1=fnval(dsp,selected_points);
% pt1=pt1';
figure;plot(a(:,1),pt');
figure;fnplt(sp,[0,4]);
hold on; plot(x1,pt1,'ro');
size(pt);
%figure;plot(a(:,1),fnval(dsp,a(:,1)));
% fp=fopen('dumbell.txt','wt');
% plot(y(:,1),y(:,2));
% for i=1:size(y,1)
%     fprintf(fp,'%f %f %d\n',y(i,1),y(i,2),b(i,3));
% end
