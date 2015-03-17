from sklearn.cluster import KMeans
from operator import itemgetter
import math

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

data=[]
for frame in frames:
	data.append(frame_to_features[frame][1:])

kmeans = KMeans(init='k-means++', n_clusters=k)
kmeans.fit(data)

print len(frames),len(kmeans.labels_)
print kmeans.labels_
print kmeans.cluster_centers_

final_feature=[]
final_feature.extend(kmeans.cluster_centers_[0])
for i in range(1,len(kmeans.cluster_centers_)):
	final_feature.extend(kmeans.cluster_centers_[i])
print final_feature

f=open('final_feature_cluster.csv','w')
s=str(final_feature[0])
for i in range(1,len(final_feature)):
	s=s+','+str(final_feature[i])
s=s+'\n'
f.write(s)
f.close()
