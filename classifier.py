from operator import itemgetter
import math

def euclid_dist(a,b):
	res=0.0
	for i in range(1,49):
		res+=(float(a[i])-float(b[i]))*(float(a[i])-float(b[i]))
	return math.sqrt(res)

def trueLabel(frame):
	frame=int(frame)
	for i in posture1:
		if frame>= i[0] and frame<= i[1]:
			return 1
	for i in posture2:
		if frame>= i[0] and frame<= i[1]:
			return 2
	for i in posture3:
		if frame>= i[0] and frame<= i[1]:
			return 3


f=open('bsplineder.txt','r')
frames=f.readlines()
posture1=[(0,94),(138,182),(222,272),(309,355),(396,439),(461,481),(504,539),(578,622),(660,677)]
posture2=[(95,137),(273,308),(440,460),(540,577)]
posture3=[(183,221),(356,395),(482,503),(623,659)]

a=dict()
for feature in open('features.csv'):
	l=feature.split(',')
	for i in range(0,len(l)):
		l[i]=l[i].rstrip()
	a[l[0]]=l

test_label=dict()

c=0
c1=0
for feature in open('features.csv'):
	l=feature.split(',')
	for i in range(0,len(l)):
		l[i]=l[i].rstrip()
	dist=[]
	for frame in frames:
                frame=frame.rstrip()
		d=euclid_dist(l,a[frame])
		dist.append((frame,d))
	dist=sorted(dist,key=itemgetter(1))
	test_label[l[0]]=trueLabel(dist[0][0])
	if test_label[l[0]]==trueLabel(l[0]):
		c1+=1
	c+=1

print (c1*100.0)/(c)	
