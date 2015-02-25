%b=importdata('data_bharat.txt');
b=importdata('data.txt');
a=b(1:350,1:2);
y=sgolayfilt(a,3,41);
fp=fopen('dumbell.txt','wt');
plot(y(:,1),y(:,2));
for i=1:size(y,1)
    fprintf(fp,'%f %f %d\n',y(i,1),y(i,2),b(i,3));
end
