a=load('C:\Users\CSE\Downloads\key_frame_clusters.csv')
dirname='Cluster0';
mkdir(dirname);
filename='E:\Natta_MatFiles_1\color_USB-VID_045E&PID_02BF-0000000000000000_';
temp=num2str(a(1,2));
filename=strcat(filename,temp);
filename=strcat(filename,'.png');
I=imread(filename);
filename=strcat(strcat(strcat(dirname,'\'),num2str(a(1,2))),'.png');
imwrite(I,filename);
for i=2:size(a,1)
    if a(i,1)~=a(i-1,1)
        dirname=strcat('Cluster',num2str(a(i,1)));
        mkdir(dirname);
    end    
    filename='E:\Natta_MatFiles_1\color_USB-VID_045E&PID_02BF-0000000000000000_';
    temp=num2str(a(i,2));
    filename=strcat(filename,temp);
    filename=strcat(filename,'.png');
    I=imread(filename);
    filename=strcat(strcat(strcat(dirname,'\'),num2str(a(i,2))),'.png');
    imwrite(I,filename);
end
