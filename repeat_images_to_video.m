aviobj = avifile('example.avi');
aviobj.fps=30;
%outputVideo = VideoWriter('out.avi');
%open(outputVideo);
frames=importdata('bsplineder.txt');
frames=sort(frames);

i=1;
j=1;
while j<length(frames)
    if i>frames(j)
        j=j+1
    end
    i=i+1
    filename='E:\Natta_MatFiles\color_USB-VID_045E&PID_02BF-0000000000000000_';
     temp=num2str(frames(j));
     filename=strcat(filename,temp);
     filename=strcat(filename,'.png');
     img = imread(filename);
    % writeVideo(outputVideo,img);
    aviobj = addframe(aviobj,img);
    
end


aviobj=close(aviobj);
