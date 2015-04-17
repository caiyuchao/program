#include <stdio.h>
#include <iostream>
#include <math.h>

using namespace std;

#define eps 1e-6

struct TPoint
{
    double x, y;
};

struct TLine
{
    TPoint p1, p2;
};

double max(double x, double y)
{
    //比较两个数的大小，返回大的数
    if(x > y) return x;
    else return y; 
}

double min(double x, double y)
{
    //比较两个数的大小，返回小的数
    if(x < y) return x;
    else return y; 
}

double multi(TPoint p1, TPoint p2, TPoint p0)
{
    //求矢量[p0, p1], [p0, p2]的叉积
    //p0是顶点 
    return (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);
    //若结果等于0，则这三点共线
    //若结果大于0，则p0p2在p0p1的逆时针方向
    //若结果小于0，则p0p2在p0p1的顺时针方向 
}

bool isIntersected(TPoint s1, TPoint e1, TPoint s2, TPoint e2)
{
    //判断线段是否相交
    //1.快速排斥试验判断以两条线段为对角线的两个矩形是否相交 
    //2.跨立试验
    if(
    (max(s1.x, e1.x) >= min(s2.x, e2.x)) &&
    (max(s2.x, e2.x) >= min(s1.x, e1.x)) &&
    (max(s1.y, e1.y) >= min(s2.y, e2.y)) &&
    (max(s2.y, e2.y) >= min(s1.y, e1.y)) &&
    (multi(s2, e1, s1) * multi(e1, e2, s1) >= 0) &&
    (multi(s1, e2, s2) * multi(e2, e1, s2) >= 0)
    )  return true;
    
    return false;    
}

int main()
{
    //freopen("in.in", "r", stdin);
    //freopen("out.out", "w", stdout);
    int ca, n, i, j, t, ltmp, qn;
    double tmp[100];
    TPoint p0, point[80];
    TPoint Q[300];
    TLine line[40]; 
    while(scanf("%d", &n) != EOF){ 
        t = 0;
        for(i = 0;i < n;i++ ){
            scanf("%lf%lf%lf%lf", &line[i].p1.x, &line[i].p1.y,
               &line[i].p2.x, &line[i].p2.y);
            point[t++] = line[i].p1;
            point[t++] = line[i].p2;
        }
        scanf("%lf%lf", &p0.x, &p0.y); 
        //x = 0这条线上的
        ltmp = 1;
        tmp[0] = 0.0;
        for(i = 0;i < t;i++){
            if(fabs(point[i].x) < eps) tmp[ltmp++] = point[i].y;
        } 
        tmp[ltmp++] = 100.0;
        sort(tmp, tmp + ltmp);
        qn = 0; 
        for(i = 1;i < ltmp;i++){
            Q[qn].x = 0.0;
            Q[qn++].y = (tmp[i - 1] + tmp[i]) / 2;
        }
        //y = 0这条线上
        ltmp = 1;
        tmp[0]= 0.0;
        for(i = 0;i < t;i++){
            if(fabs(point[i].y) < eps) tmp[ltmp++] = point[i].x;
        }
        tmp[ltmp++] = 100.0;
        sort(tmp, tmp + ltmp);
        for(i = 1;i < ltmp;i++){
            Q[qn].x = (tmp[i - 1] + tmp[i]) / 2;
            Q[qn++].y = 0.0;
        }
        //x = 100 这条线上
        ltmp = 1;
        tmp[0] = 0.0;
        for(i = 0;i < t;i++){
            if(fabs(point[i].x - 100) < eps) tmp[ltmp++] = point[i].y;
        }
        tmp[ltmp++] = 100.0;
        sort(tmp, tmp + ltmp);
        for(i = 1;i < ltmp;i++){
            Q[qn].x = 100.0;
            Q[qn++].y = (tmp[i - 1] + tmp[i]) / 2;
        }
        //y = 100这条线
        ltmp = 1;
        tmp[0] = 0.0;
        for(i = 0;i < t;i++){
            if(fabs(point[i].y - 100.0) < eps) tmp[ltmp++] = point[i].x;
        } 
        tmp[ltmp++] = 100.0;
        sort(tmp, tmp + ltmp);
        for(i = 1;i < ltmp;i++){
            Q[qn].x = (tmp[i - 1] + tmp[i]) / 2;
            Q[qn++].y = 100.0;
        }
        int ans = 9999999, ans1;
        for(i = 0;i < qn;i++){
            ans1 = 0;
            for(j = 0;j < n;j++){
                if(isIntersected(line[j].p1, line[j].p2, Q[i], p0)) ans1++;
            }
            if(ans1 < ans) ans = ans1;   
        }
        printf("Number of doors = %d\n", ans + 1);
    }  
    return 0;    
}
