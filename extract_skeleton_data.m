%USB-VID_045E&PID_02BF-0000000000000000_13
function BX=extract_skelton_data()
    flag=1;
    for i=1:531
       %filename='H:\Adavus and postures\another1\export\USB-VID_045E&PID_02BF-0000000000000000_'
       %filename='H:\Adavus and postures\good\export\export2\USB-VID_045E&PID_02BF-0000000000000000_'
       %filename='H:\Adavus and postures\abhishek1\export\USB-VID_045E&PID_02BF-0000000000000000_';
       filename='H:\Adavus and postures\abhishek1\export\USB-VID_045E&PID_02BF-0000000000000000_'
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
    output_file=strcat('abhishek1.txt');
    f1=fopen(output_file,'w');
    for k=i:531
       if k==14 | k==107 | k==167 | k==191
        filename='H:\Adavus and postures\abhishek1\export\USB-VID_045E&PID_02BF-0000000000000000_'
        temp=num2str(k);
        filename=strcat(filename,temp);
        A=load(filename);
        
        
        for l=1:6
            if strcmp(A.SkeletonFrame.Skeletons(l).TrackingState,'Tracked')==1
              break;  
            end    
        end
        
  
        calculate_ratios(A.SkeletonFrame.Skeletons(l),f1,k);
          
        
%         if k==122
%             calculate_ratios(A.SkeletonFrame.Skeletons(l),f1,k);
%         end
%         
%         if k==148
%             calculate_ratios(A.SkeletonFrame.Skeletons(l),f1,k);
%         end
%         
%         if k=203
%             calculate_ratios(A.SkeletonFrame.Skeletons(l),f1,k);
%         end
%         
%         if k==287
%             calculate_ratios(A.SkeletonFrame.Skeletons(l),f1,k);
%         end
%         
%         if k==340
%             calculate_ratios(A.SkeletonFrame.Skeletons(l),f1,k);
%         end
%         
%         if k==343
%             calculate_ratios(A.SkeletonFrame.Skeletons(l),f1,k);
%         end
%         
%         if k==348
%             calculate_ratios(A.SkeletonFrame.Skeletons(l),f1,k);
%         end
        
%         fclose(f1);
       end
       
    end    
    


end

function calculate_ratios(A,f1,k)
    %ratio1
    x1=dist(A.Joints(5).Position,A.Joints(3).Position);
    x2=dist(A.Joints(3).Position,A.Joints(9).Position);
    l1=x1+x2; % length of two shoulder bones
    
    x1=dist(A.Joints(3).Position,A.Joints(5).Position);
    x2=dist(A.Joints(5).Position,A.Joints(6).Position);
    x3=dist(A.Joints(6).Position,A.Joints(7).Position);
    x4=dist(A.Joints(7).Position,A.Joints(8).Position);
    l21=x1+x2+x3+x4 %left hand total length
    
    x1=dist(A.Joints(3).Position,A.Joints(9).Position);
    x2=dist(A.Joints(9).Position,A.Joints(10).Position);
    x3=dist(A.Joints(10).Position,A.Joints(11).Position);
    x4=dist(A.Joints(11).Position,A.Joints(12).Position);
    l22=x1+x2+x3+x4 %lright hand total length
    
    x1=dist(A.Joints(4).Position,A.Joints(3).Position);
    x2=dist(A.Joints(3).Position,A.Joints(2).Position);
    x3=dist(A.Joints(2).Position,A.Joints(1).Position);
    x4=dist(A.Joints(1).Position,A.Joints(13).Position);
    x5=dist(A.Joints(13).Position,A.Joints(14).Position);
    x6=dist(A.Joints(14).Position,A.Joints(15).Position);
    x7=dist(A.Joints(15).Position,A.Joints(16).Position);
    l31=x1+x2+x3+x4+x5+x6+x7; %total body height
    
    x1=dist(A.Joints(4).Position,A.Joints(3).Position);
    x2=dist(A.Joints(3).Position,A.Joints(2).Position);
    x3=dist(A.Joints(2).Position,A.Joints(1).Position);
    x4=dist(A.Joints(1).Position,A.Joints(17).Position);
    x5=dist(A.Joints(17).Position,A.Joints(18).Position);
    x6=dist(A.Joints(18).Position,A.Joints(19).Position);
    x7=dist(A.Joints(19).Position,A.Joints(20).Position);
    l32=x1+x2+x3+x4+x5+x6+x7; %total body height
    
    r1=l1/l31;
    r2=l1/l32;
    
    r3=l1/l21;
    r4=l1/l22;
    
    r5=l21/l31;
    r6=l22/l32;
    
    fprintf(f1,'%d %f %f %f %f %f %f \n',k,r1,r2,r3,r4,r5,r6);
end

function y=dist(A1,A2)
    X1=(A1.X-A2.X)*(A1.X-A2.X);
    Y1=(A1.Y-A2.Y)*(A1.Y-A2.Y);
    Z1=(A1.Z-A2.Z)*(A1.Z-A2.Z);
    y=sqrt(X1+Y1+Z1);
end
