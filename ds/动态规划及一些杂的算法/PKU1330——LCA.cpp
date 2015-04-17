//Source Code-17 LCA- Tarjn

// pk1330.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <memory.h>

#define MAXN 10001
#define MAXL 100000
int root[MAXN],head[MAXN],data[MAXL],next[MAXL],ans[MAXN],q[MAXN];
bool flag[MAXN];
char ch;
int n,top,ro;

void add(int x,int y){
	data[top]=y;
	next[top]=head[x];
	head[x]=top++;
}

void add2(int x,int y){
	data[top]=y;
	next[top]=q[x];
	q[x]=top++;
}

void init(){
	int i,a,b;
	scanf("%d",&n);
	memset(head,-1,sizeof(head));
	memset(q,-1,sizeof(q));
	memset(flag,false,sizeof(flag));
	memset(ans,0,sizeof(ans));
	top=0;
	for(i=1;i<n;i++){
		scanf("%d %d",&a,&b);
		add(a,b);
		flag[b]=true;
	}
	for(i=1;i<=n;i++)
		if(!flag[i]){
			ro=i;
			break;
		}
		memset(flag,false,sizeof(flag));
		scanf("%d %d",&a,&b);
		add2(a,b);
		add2(b,a);
}
int find(int x){
	if(root[x]!=x) root[x]=find(root[x]);
	return root[x];
}
void lca(int v){
	int p;
	root[v]=v;
	for(p=head[v];p>=0;p=next[p]){
		lca(data[p]);
		root[data[p]]=v;
	}
	flag[v]=true;
	for(p=q[v];p>=0;p=next[p])
		if(flag[data[p]]){
			ans[find(data[p])]++;
			int j=p;
		}
}
void print(){
	for(int i=1;i<=n;i++)
		if(ans[i]>0) 
			break;
		i=i;
		printf("%d\n",i);
}
int main(int argc, char* argv[])
{
	int t;
	scanf("%d",&t);
	while(t--){
		init();
		lca(ro);
		print();
	}
	return 0;
}
