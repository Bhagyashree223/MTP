function BX=right_hand_coordinates()
    flag=1;
    for i=1:678
       %filename='H:\Adavus and postures\another1\export\USB-VID_045E&PID_02BF-0000000000000000_'
       %filename='H:\Adavus and postures\good\export\export2\USB-VID_045E&PID_02BF-0000000000000000_'
       %filename='H:\Adavus and postures\abhishek1\export\USB-VID_045E&PID_02BF-0000000000000000_';
      
       %filename='H:\Adavus and postures\abhishek1\export\USB-VID_045E&PID_02BF-0000000000000000_'
       filename='E:\Natta_MatFiles\USB-VID_045E&PID_02BF-0000000000000000_'
       temp=num2str(i);
       filename=strcat(filename,temp);
       A=load(filename);
       fprintf('filename : %s\n',filename);
       %disp(filename);
       for j=1:6
          if strcmp(A.SkeletonFrame.Skeletons(j).TrackingState,'Tracked')==1
             flag=0; 
             break; 
          end    
       end    
       if flag==0
           break;
       end    
%        if i==50
%            [m n]=size(X);
%            BX = zeros(m, n);
%            f1=fopen('temp.txt','w');
%             for j=1:m
%                for k=1:n
%                   fprintf(f1,'%d,%d,%d\n',X(j,k),Y(j,k),Z(j,k)); 
%                end    
%             end    
% 
%        end

        
    end
    output_file1=strcat('data_xy.txt');
    f1=fopen(output_file1,'w');
	output_file2=strcat('data_yz.txt');
    f2=fopen(output_file2,'w');
	output_file3=strcat('data_zx.txt');
    f3=fopen(output_file3,'w');
    for k=i:531
	
        filename='E:\Natta_MatFiles\USB-VID_045E&PID_02BF-0000000000000000_'
        temp=num2str(k);
        filename=strcat(filename,temp);
        A=load(filename);
        
        for l=1:6
            if strcmp(A.SkeletonFrame.Skeletons(l).TrackingState,'Tracked')==1
              break;
            end    
        end
        
		fprintf(f1,'%f %f %d\n',A.SkeletonFrame.Skeletons(l).Joints(12).Position.X,A.SkeletonFrame.Skeletons(l).Joints(12).Position.Y,k);
       % A.SkeletonFrame.Skeletons(l).Joints(12).Position.X,A.SkeletonFrame.Skeletons(l).Joints(12).Position.Y,k;
        fprintf(f2,'%f %f %d\n',A.SkeletonFrame.Skeletons(l).Joints(12).Position.Y,A.SkeletonFrame.Skeletons(l).Joints(12).Position.Z,k);
		fprintf(f3,'%f %f %d\n',A.SkeletonFrame.Skeletons(l).Joints(12).Position.Z,A.SkeletonFrame.Skeletons(l).Joints(12).Position.X,k);
    end  
    %fclose(f1);fclose(f2);fclose(f3);
end
