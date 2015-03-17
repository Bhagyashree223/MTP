from operator import itemgetter
import math

def euclid_dist(a,b):
	res=0.0
	for i in range(1,49):
		res+=(float(a[i])-float(b[i]))*(float(a[i])-float(b[i]))
	return math.sqrt(res)


k=20

frames=open('bsplineder.txt','r').readlines()
for i in range(0,len(frames)):
	frames[i]=frames[i].rstrip()

frame_to_features=dict()
for feature in open('features.csv','r'):
	l=feature.split(',')
	for i in range(0,len(l)):
		l[i]=l[i].rstrip()
	frame_to_features[l[0]]=l

a=[]
for i in range(0,len(frames)):
	for j in range(i+1,len(frames)):
		a.append( ( (frames[i],frames[j]) , euclid_dist( frame_to_features[frames[i]], frame_to_features[frames[j]] ) )  )

a=sorted(a,key=itemgetter(1),reverse=True)
top_k=[]
for i in a:
	print i
	if i[0][0] not in top_k:
		top_k.append(i[0][0])
		k-=1
	if k==0:
		break
	if i[0][1] not in top_k:
		top_k.append(i[0][1])
		k-=1
	if k==0:
		break	

print top_k	

final_feature=[]
for i in top_k:
	print frame_to_features[i][1:]
	final_feature.extend(frame_to_features[i][1:])



f=open('final_feature_max_dist.csv','w')
s=str(final_feature[0])
for i in range(1,len(final_feature)):
	s=s+','+str(final_feature[i])
s=s+'\n'
f.write(s)
f.close()



