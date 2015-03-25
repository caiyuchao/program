#include<stdio.h>
#include<string.h>
int n, m;
class bigInt
{ 
public: 
    int num[200], len; 
    bigInt() { num[0] = 0, len = 0; }
    bigInt operator=(const int &a) 
    { 
        int tmp = a; 
        len = 0; 
        while (tmp) 
            num[len++] = tmp % 10, tmp /= 10; 
        if (!len) num[0] = 0, len = 1; 
    } 
    bool operator<(const bigInt &a) 
    { 
        if (a.len != len) 
            return len < a.len; 
        for (int i = len - 1; i >= 0; i--) 
            if (num[i] != a.num[i]) 
                return num[i] < a.num[i]; 
        return false; 
    } 
    bigInt operator+(const bigInt &a) 
    { 
        bigInt res; 
        int i, j, c = 0, adda, addb; 
        for (i = 0, j = 0; i < len || j < a.len || c; ) 
        { 
            adda = 0, addb = 0; 
            if (i < len) 
                adda = num[i++]; 
            if (j < a.len) 
                addb = a.num[j++]; 
            res.num[res.len++] = (adda + addb + c) % 10; 
            c = (adda + addb + c) / 10; 
        } 
        return res; 
    } 
    bigInt operator-(const bigInt &b) 
    { 
        bigInt res; 
        int i, j, c = 0, suba, subb; 
        for (i = 0, j = 0; i < len || j < b.len || c; ) 
        { 
            suba = 0, subb = 0; 
            if (i < len) 
                suba = num[i++]; 
            if (j < b.len) 
                subb = b.num[j++]; 
            res.num[res.len++] = (suba - subb + c + 10) % 10; 
            c = (suba - subb + c + 10) / 10 - 1; 
        } 
        for (i = res.len - 1; i > 0; i--) 
            if (res.num[i]) break; 
        res.len = i + 1; 
        return res; 
    } 
    bigInt operator*(const bigInt &b) 
    { 
        bigInt res; 
        int i, j, c, now, mulb, tmp; 
        memset(res.num, 0, sizeof(int) * (len + b.len)); 
        for (i = 0; i < len; i++) 
        { 
            now = i, c = 0; 
            for (j = 0; j < b.len || c; ) 
            { 
                mulb = 0; 
                if (j < b.len) 
                    mulb = b.num[j++]; 
                tmp = res.num[now] + num[i] * mulb + c; 
                res.num[now++] = tmp % 10; 
                c = tmp / 10; 
            } 
        } 
        for (i = len + b.len - 1; i > 0; i--) 
            if (res.num[i]) break; 
        res.len = i + 1; 
        return res; 
    } 
    bigInt operator/(const bigInt &b) 
    { 
        bigInt res, diva; 
        int i, j, c; 
        for (i = len - 1; i >= 0; i--) 
        { 
            if (diva.len > 1 || diva.num[0]) 
            { 
                for (j = diva.len - 1; j >= 0; j--) 
                    diva.num[j + 1] = diva.num[j]; 
                diva.len++; 
            } 
            diva.num[0] = num[i]; 
            if (!diva.len) diva.len = 1; 
            res.num[i] = 0; 
            while (!(diva < b)) 
                diva = diva - b, res.num[i]++; 
        } 
        for (i = len - 1; i > 0; i--) 
            if (res.num[i]) break; 
        res.len = i + 1; 
        return res; 
    } 
    bigInt operator%(const bigInt &b) 
    { 
        bigInt res, diva; 
        int i, j, c; 
        for (i = len - 1; i >= 0; i--) 
        { 
            if (diva.len > 1 || diva.num[0]) 
            { 
                for (j = diva.len - 1; j >= 0; j--) 
                    diva.num[j + 1] = diva.num[j]; 
                diva.len++; 
            } 
            diva.num[0] = num[i]; 
            if (!diva.len) diva.len = 1; 
            res.num[i] = 0; 
            while (!(diva < b)) 
                diva = diva - b, res.num[i]++; 
        } 
        for (i = diva.len - 1; i > 0; i--) 
            if (diva.num[i]) break; 
        diva.len = i + 1; 
        return diva; 
    } 
    void display() 
    { 
        int i; 
        for (i = len - 1; i > 1; i--) 
            if (num[i]) break; 
        for (; i >= 0; i--) 
            printf("%d", num[i]); 
    } 
}; 
