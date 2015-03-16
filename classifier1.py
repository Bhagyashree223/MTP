from operator import itemgetter
import math

def euclid_dist(a,b):
	res=0.0
	for i in range(1,49):
		res+=(float(a[i])-float(b[i]))*(float(a[i])-float(b[i]))
	return math.sqrt(res)

def trueLabel(frame):
	for i in posture1:
		if int(frame)>= i[0] and int(frame)<= i[1]:
			return 1
	for i in posture2:
		if int(frame)>= i[0] and int(frame)<= i[1]:
			return 2
	for i in posture3:
		if int(frame)>= i[0] and int(frame)<= i[1]:
			return 3


f=open('bsplineder.txt','r')
frames=f.readlines()
posture1=[(0,93),(131,175),(213,256),(297,344),(385,428),(465,508),(548,588),(627,674),(709,753)]
posture2=[(94,130),(257,296),(429,464),(589,626)]
posture3=[(176,212),(345,384),(509,547),(675,708)]

a=dict()
for feature in open('features.csv'):
	l=feature.split(',')
	a[l[0]]=l

test_label=dict()

c=0
c1=0
for feature in open('features.csv'):
	l=feature.split(',')
	dist=[]
	for frame in frames:
                frame=frame.rstrip()
		d=euclid_dist(l,a[frame])
		dist.append((frame,d))
	dist=sorted(dist,key=itemgetter(1))
	test_label[l[0]]=trueLabel(dist[0][0])
	#print l[0]
	print test_label[l[0]],trueLabel(l[0])
	if test_label[l[0]]==trueLabel(l[0]):
		c1+=1
	c+=1

print (c1*100.0)/(c)	
