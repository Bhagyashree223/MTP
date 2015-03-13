aviobj = avifile('example.avi');
aviobj.fps=5;
%outputVideo = VideoWriter('out.avi');
%open(outputVideo);
frames=importdata('bsplineder.txt');
frames=sort(frames)

for i = 1:length(frames)
     filename='E:\Natta_MatFiles\color_USB-VID_045E&PID_02BF-0000000000000000_';
     temp=num2str(frames(i));
     filename=strcat(filename,temp);
     filename=strcat(filename,'.png');
     img = imread(filename);
    % writeVideo(outputVideo,img);
    aviobj = addframe(aviobj,img); 
end

aviobj=close(aviobj);
