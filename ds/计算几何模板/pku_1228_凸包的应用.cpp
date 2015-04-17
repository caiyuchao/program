//pku_1228_凸包的应用.cpp
#include <stdio.h> 
#include <math.h>
#include <stdlib.h> 

#define MaxNode 10005 
#define eps 1e-6

struct TPoint 
{ 
    double x, y;
};

TPoint pp; 

void swap(TPoint point[], int i, int j) 
{ 
    TPoint tmp; 
    tmp = point[i]; 
    point[i] = point[j]; 
    point[j] = tmp; 
} 

double multi(TPoint p1, TPoint p2, TPoint p0) 
{ 
    return (p1.x - p0.x) * (p2.y - p0.y) 
         - (p2.x - p0.x) * (p1.y - p0.y); 
} 

double distance(TPoint p1, TPoint p2) 
{ 
    double  tmp = (p1.x - p2.x) * (p1.x - p2.x) 
        + (p1.y - p2.y) * (p1.y - p2.y); 
    return sqrt(tmp);
} 

int cmp(const void *a, const void *b) 
{ 
    TPoint *c = (TPoint *)a; 
    TPoint *d = (TPoint *)b; 
    double k = multi(*c, *d, pp); 
    if(k < 0) return 1; 
    else if(fabs(k) < eps && distance(*c, pp) >= distance(*d, pp)) return 1; 
    else return -1;    
} 

void grahamScan(TPoint point[], int stack[], int &top, int n) 
{  
    int i, u;      
    u = 0; 
    for(i = 1;i <= n - 1;i++){ 
        if((point[i].y < point[u].y) || (point[i].y == point[u].y 
            && point[i].x  < point[u].x)) 
        u = i;       
    }  
    swap(point, 0, u); 
    pp = point[0];
    qsort(point + 1, n - 1, sizeof(point[0]), cmp); 
    for(i = 0;i <= 1;i++) stack[i] = i; 
    top = 1; 
    for(i = 2;i <= n - 1;i++){ 
        while(multi(point[i], point[stack[top]], point[stack[top - 1]]) >= 0){ 
            if(top == 0)break; 
            top--; 
        } 
        top++; 
        stack[top] = i; 
    } 
} 

int InLine(TPoint p, TPoint p1, TPoint p2)
{
    double d1, d2, d3;
    d1 = distance(p1, p);
    d2 = distance(p, p2);
    d3 = distance(p1, p2);
    if(fabs(d1 + d2 - d3) < eps) return 1;
    else return 0; 
}

int check(TPoint point[], int n, TPoint p[], int pn) 
{
    int num, i, j;
    point[n] = point[0];
    p[pn] = p[0];
    for(j = 0;j < pn;j++){
        num = 0;
        for(i = 0;i < n;i++){
            if(InLine(point[i], p[j], p[j + 1])) num++;
        }
        if(num < 3) return 0;
    }
    return 1;
}

double Area(TPoint p1, TPoint p2, TPoint p3)
{
    TPoint t1, t2;
    t1.x = p2.x - p1.x;
    t1.y = p2.y - p1.y;
    t2.x = p3.x - p1.x;
    t2.y = p3.y - p1.y;
    return fabs(t1.x * t2.y - t1.y * t2.x);
}

int main() 
{ 
    int stack[MaxNode], top, i, pn, n, tmp, test; 
    TPoint point[MaxNode], p[MaxNode]; 
    scanf("%d", &test); 
    while(test--){ 
        scanf("%d", &n); 
        for(i = 0;i < n;i++) {
            scanf("%lf%lf", &point[i].x, &point[i].y); 
        }
        double area = 0.0;
        for(i = 1;i < n - 1;i++){
            area += Area(point[0], point[i], point[i + 1]);
        }
        if(fabs(area) < eps){
            printf("NO\n");
            continue;
        } 
        grahamScan(point, stack, top, n);  
        for(i = 0;i <= top;i++){
            p[i] = point[stack[i]];
        }
        pn = top + 1;
        tmp = check(point, n, p, pn);
        if(tmp) printf("YES\n");
        else printf("NO\n");
    } 
    return 0; 
} 
