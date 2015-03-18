####merge two bsplineder.txt files containing selected frames from tracking left and right hand joints 

l1=open('bsplineder.txt','r').readlines()
l2=open('bsplineder1.txt','r').readlines()

for i in range(0,len(l1)):
	l1[i]=l1[i].rstrip()
for i in range(0,len(l2)):
	l2[i]=l2[i].rstrip()

l=list(set(l1) | set(l2))
l=sorted(l)
f=open('final_bsplineder.txt','w')
s=""
for i in l:
	s+=str(i)+"\n"
print s
f.write(s)
f.close()
