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
    output_file1=strcat('features.csv');
    f1=fopen(output_file1,'w');
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

    //above center
    ax1=A.SkeletonFrame.Skeletons(l).Joints(3).Position.X;ay1=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Y;az1=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;
    
    //above left
    ax2=A.SkeletonFrame.Skeletons(l).Joints(5).Position.X;ay2=A.SkeletonFrame.Skeletons(l).Joints(5).Position.Y;az2=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;
    ax3=A.SkeletonFrame.Skeletons(l).Joints(6).Position.X;ay3=A.SkeletonFrame.Skeletons(l).Joints(6).Position.Y;az3=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;
    ax4=A.SkeletonFrame.Skeletons(l).Joints(7).Position.X;ay4=A.SkeletonFrame.Skeletons(l).Joints(7).Position.Y;az4=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;
    ax5=A.SkeletonFrame.Skeletons(l).Joints(8).Position.X;ay5=A.SkeletonFrame.Skeletons(l).Joints(8).Position.Y;az5=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;

    //above right
    ax6=A.SkeletonFrame.Skeletons(l).Joints(9).Position.X;ay6=A.SkeletonFrame.Skeletons(l).Joints(9).Position.Y;az6=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;
    ax7=A.SkeletonFrame.Skeletons(l).Joints(10).Position.X;ay7=A.SkeletonFrame.Skeletons(l).Joints(10).Position.Y;az7=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;
    ax8=A.SkeletonFrame.Skeletons(l).Joints(11).Position.X;ay8=A.SkeletonFrame.Skeletons(l).Joints(11).Position.Y;az8=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;
    ax9=A.SkeletonFrame.Skeletons(l).Joints(12).Position.X;ay9=A.SkeletonFrame.Skeletons(l).Joints(12).Position.Y;az9=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;

   //below center
    bx1=A.SkeletonFrame.Skeletons(l).Joints(1).Position.X;by1=A.SkeletonFrame.Skeletons(l).Joints(1).Position.Y;bz1=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;
    
    //below left
    bx2=A.SkeletonFrame.Skeletons(l).Joints(13).Position.X;by2=A.SkeletonFrame.Skeletons(l).Joints(13).Position.Y;bz2=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;
    bx3=A.SkeletonFrame.Skeletons(l).Joints(14).Position.X;by3=A.SkeletonFrame.Skeletons(l).Joints(14).Position.Y;bz3=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;
    bx4=A.SkeletonFrame.Skeletons(l).Joints(15).Position.X;by4=A.SkeletonFrame.Skeletons(l).Joints(15).Position.Y;bz4=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;
    bx5=A.SkeletonFrame.Skeletons(l).Joints(16).Position.X;by5=A.SkeletonFrame.Skeletons(l).Joints(16).Position.Y;bz5=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;

    //below right
    bx6=A.SkeletonFrame.Skeletons(l).Joints(17).Position.X;by6=A.SkeletonFrame.Skeletons(l).Joints(17).Position.Y;bz6=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;
    bx7=A.SkeletonFrame.Skeletons(l).Joints(18).Position.X;by7=A.SkeletonFrame.Skeletons(l).Joints(18).Position.Y;bz7=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;
    bx8=A.SkeletonFrame.Skeletons(l).Joints(19).Position.X;by8=A.SkeletonFrame.Skeletons(l).Joints(19).Position.Y;bz8=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;
    bx9=A.SkeletonFrame.Skeletons(l).Joints(20).Position.X;by9=A.SkeletonFrame.Skeletons(l).Joints(20).Position.Y;bz9=A.SkeletonFrame.Skeletons(l).Joints(3).Position.Z;

    //xy
    angle_above_left_xy1=angle([ax1,ay1],[ax2,ay2]);
    angle_above_left_xy2=angle([ax2,ay2],[ax3,ay3]);
    angle_above_left_xy3=angle([ax3,ay3],[ax4,ay4]);
    angle_above_left_xy4=angle([ax4,ay4],[ax5,ay5]);

    angle_above_right_xy1=angle([ax1,ay1],[ax6,ay6]);
    angle_above_right_xy2=angle([ax6,ay6],[ax7,ay7]);
    angle_above_right_xy3=angle([ax7,ay7],[ax8,ay8]);
    angle_above_right_xy4=angle([ax8,ay8],[ax9,ay9]);

    angle_below_left_xy1=angle([bx1,by1],[bx2,by2]);
    angle_below_left_xy2=angle([bx2,by2],[bx3,by3]);
    angle_below_left_xy3=angle([bx3,by3],[bx4,by4]);
    angle_below_left_xy4=angle([bx4,by4],[bx5,by5]);

    angle_below_right_xy1=angle([bx1,by1],[bx6,by6]);
    angle_below_right_xy2=angle([bx6,by6],[bx7,by7]);
    angle_below_right_xy3=angle([bx7,by7],[bx8,by8]);
    angle_below_right_xy4=angle([bx8,by8],[bx9,by9]);

    //yz
    angle_above_left_yz1=angle([ay1,az1],[ay2,az2]);
    angle_above_left_yz2=angle([ay2,az2],[ay3,az3]);
    angle_above_left_yz3=angle([ay3,az3],[ay4,az4]);
    angle_above_left_yz4=angle([ay4,az4],[ay5,az5]);

    angle_above_right_yz1=angle([ay1,az1],[ay6,az6]);
    angle_above_right_yz2=angle([ay6,az6],[ay7,az7]);
    angle_above_right_yz3=angle([ay7,az7],[ay8,az8]);
    angle_above_right_yz4=angle([ay8,az8],[ay9,az9]);

    angle_below_left_yz1=angle([by1,bz1],[by2,bz2]);
    angle_below_left_yz2=angle([by2,bz2],[by3,bz3]);
    angle_below_left_yz3=angle([by3,bz3],[by4,bz4]);
    angle_below_left_yz4=angle([by4,bz4],[by5,bz5]);

    angle_below_right_yz1=angle([by1,bz1],[by6,bz6]);
    angle_below_right_yz2=angle([by6,bz6],[by7,bz7]);
    angle_below_right_yz3=angle([by7,bz7],[by8,bz8]);
    angle_below_right_yz4=angle([by8,bz8],[by9,bz9]);

    //zx
    angle_above_left_zx1=angle([az1,ax1],[az2,ax2]);
    angle_above_left_zx2=angle([az2,ax2],[az3,ax3]);
    angle_above_left_zx3=angle([az3,ax3],[az4,ax4]);
    angle_above_left_zx4=angle([az4,ax4],[az5,ax5]);

    angle_above_right_zx1=angle([az1,ax1],[az6,ax6]);
    angle_above_right_zx2=angle([az6,ax6],[az7,ax7]);
    angle_above_right_zx3=angle([az7,ax7],[az8,ax8]);
    angle_above_right_zx4=angle([az8,ax8],[az9,ax9]);

    angle_below_left_zx1=angle([bz1,bx1],[bz2,bx2]);
    angle_below_left_zx2=angle([bz2,bx2],[bz3,bx3]);
    angle_below_left_zx3=angle([bz3,bx3],[bz4,bx4]);
    angle_below_left_zx4=angle([bz4,bx4],[bz5,bx5]);

    angle_below_right_zx1=angle([bz1,bx1],[bz6,bx6]);
    angle_below_right_zx2=angle([bz6,bx6],[bz7,bx7]);
    angle_below_right_zx3=angle([bz7,bx7],[bz8,bx8]);
    angle_below_right_zx4=angle([bz8,bx8],[bz9,bx9]);
		fprintf(f1,'%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f \n',
      angle_above_left_xy1,angle_above_left_xy2,angle_above_left_xy3,angle_above_left_xy4,angle_above_right_xy1,angle_above_right_xy2,angle_above_right_xy3,angle_above_right_xy4,
      angle_below_left_xy1,angle_below_left_xy2,angle_below_left_xy3,angle_below_left_xy4,angle_below_right_xy1,angle_below_right_xy2,angle_below_right_xy3,angle_below_right_xy4,
      angle_above_left_yz1,angle_above_left_yz2,angle_above_left_yz3,angle_above_left_yz4,angle_above_right_yz1,angle_above_right_yz2,angle_above_right_yz3,angle_above_right_yz4,
      angle_below_left_yz1,angle_below_left_yz2,angle_below_left_yz3,angle_below_left_yz4,angle_below_right_yz1,angle_below_right_yz2,angle_below_right_yz3,angle_below_right_yz4,
      angle_above_left_zx1,angle_above_left_zx2,angle_above_left_zx3,angle_above_left_zx4,angle_above_right_zx1,angle_above_right_zx2,angle_above_right_zx3,angle_above_right_zx4,
      angle_below_left_zx1,angle_below_left_zx2,angle_below_left_zx3,angle_below_left_zx4,angle_below_right_zx1,angle_below_right_zx2,angle_below_right_zx3,angle_below_right_zx4);
       % A.SkeletonFrame.Skeletons(l).Joints(12).Position.X,A.SkeletonFrame.Skeletons(l).Joints(12).Position.Y,k;
    end  
    %fclose(f1);fclose(f2);fclose(f3);
end

function agle=angle(a,b)
  
end
