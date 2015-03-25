//阶乘各算法的 C++ 类实现
#include <iostream>
#include <cstring>
#include <iomanip> 
#include <cmath> 
using namespace std;
 
class Factorial {
    static const int MAXN = 5001;   // 最大阶乘数，实际用不到这么大 
    int *data[MAXN];                // 存放各个数的阶乘
    int *nonzero;                   // 从低位数起第一个非0数字 
    int maxn;                       // 存放最大已经计算好的n的阶乘 
    int SmallFact(int n);            // n <= 12的递归程序 
    void TransToStr(int n, int *s);  // 将数n倒序存入数组中 
    void Multply (int* A, int* B, int* C, int totallen);   
    // 执行两个高精度数的乘法
public:
    Factorial();
    ~Factorial();
    void Calculate(int n);          // 调用计算阶乘 
    int FirstNonZero(int n);        // 返回阶乘末尾第一个非0数字 
    int CountZeros(int n);          // 返回阶乘末尾有多少个0 
    int SecondNum(int n);           // 返回阶乘左边的第二个数字 
    bool CanDivide(int m, int n);   // 判断数值 m 是否可以整除 n! 
    void Output(int n) const;
}; 
   
int Factorial::SmallFact(int n) {
    if (n == 1 || n == 0) return 1;
    return SmallFact(n-1)*n;
}    
 
void Factorial::TransToStr(int n, int *tmp) {
    int i = 1;
    while (n) {
        tmp[i++] = n%10;
        n /= 10;
    }    
    tmp[0] = i-1;
}   
 
void Factorial::Multply (int* A, int* B, int* C, int totallen) 
{
    int i, j, len;    
    memset(C, 0, totallen*sizeof(int));    
    for (i = 1; i <= A[0]; i++)
    for (j = 1; j <= B[0]; j++) {
        C[i+j-1] += A[i]*B[j];       
        // 当前i+j-1位对应项 + A[i] * B[j]
        C[i+j] += C[i+j-1]/10;       
        // 它的后一位 + 它的商（进位） 
        C[i+j-1] %= 10;              
        // 它再取余即可 
    }
    len = A[0] + B[0];    
    while (len > 1 && C[len] == 0 ) len--; 
    // 获得它的实际长度 
    C[0] = len;   
}  
 
Factorial::Factorial() {          
    // 构造函数，先把<=12的阶乘计算好 
    maxn = 12; data[0] = new int [2];
    data[0][0] = 1; data[0][1] = 1;
    int i, j = 1;
    for (i = 1; i <= 12; i++) {
        data[i] = new int [12];
        j = j*i;
        TransToStr(j, data[i]);
    }    
    nonzero = new int [10*MAXN];
    nonzero[0] = 1; nonzero[1] = 1; 
    // nonzero[0]存储已经计算到的n!末尾非0数 
}
 
Factorial::~Factorial() {
    for (int i = 0; i <= maxn; i++)
        delete [] data[i];
    delete [] nonzero; 
}    
 
void Factorial::Calculate(int n) {
    if (n > MAXN)  return;     
    if (n <= maxn) return;    
    // <= maxn的，已经在计算好的数组中了 
    int i, j, len;
    int tmp[12];
    for (i = maxn+1; i <= n; i++) {
        TransToStr(i, tmp);
        len = data[i-1][0] + tmp[0] + 1;
        data[i] = new int [len+1];
        Multply(data[i-1], tmp, data[i], len+1);
    }
    maxn = n;
}
 
int Factorial::FirstNonZero(int n) {
    if (n >= 10*MAXN) {
        cout << "Super Pig, your input is so large, cannot Calculate. Sorry!\n";
        return -1;
    }     
    if (n <= nonzero[0]) return nonzero[n];      
    //已经计算好了，直接返回

    int res[5][4] = {{0,0,0,0}, {2,6,8,4}, {4,2,6,8}, {6,8,4,2}, {8,4,2,6}};
    int i, five, t;
    for (i = nonzero[0]+1; i <= n; i++) {
        t = i;
        while (t%10 == 0) t /= 10;                  
        // 先去掉 i 末尾的 0，这是不影响的 
        if (t%2 == 0) {                                   
            // t是偶数直接乘再取模10即可 
            nonzero[i] = (nonzero[i-1]*t)%10; 
        }     
        else {       // 否则转换成 res 数组来求 
            five = 0;
            while (t%5 == 0) {
                if (five == 3) five = 0;
                else five++;
                t /= 5; 
            }     
            nonzero[i] = res[((nonzero[i-1]*t)%10)/2][five];
            // (nonzero[i-1]*t)%10/2 正好序号为：1, 2, 3, 4 中的一个 
        }     
    } 
    nonzero[0] = n; 
    return nonzero[n]; 
}  
 
/* 阶乘末尾有多少个0，实际上只与5的因子数量有关，
即求 n/5+n/25+n/625+...... */
int Factorial::CountZeros(int n) {
    if (n >= 2000000000) {
        cout << "Super Pig, your input is so large, cannot Calculate. Sorry!\n";
        return -1;
    }   
    int cnt = 0;
    while (n) {
        n /= 5;
        cnt += n;
    }
    return cnt; 
} 
 
/*  输出N!左边第二位的数字：用实数乘，
超过100就除以10，最后取个位即可  */
int Factorial::SecondNum(int n) {
    if (n <= 3) return 0; 
    int i; 
    double x = 6;
    for (i = 4; i <= n; i++) {
        x *= i;
        while (x >= 100) x /= 10; 
    }
    return (int(x))%10; 
}     
 
bool Factorial::CanDivide(int m, int n) {
    if (m == 0) return false;
    if (n >= m) return true; 
    int nn, i, j, nums1, nums2;
    bool ok = true; 
    j = (int)sqrt(1.0*m);
    for (i = 2; i <= j; i++) {
        if (m%i == 0) {
            nums1 = 0;      // 除数m的素因子i的数量 
            while (m%i == 0) {
                nums1++;
                m /= i;
            } 
            nums2 = 0; nn = n; 
            while (nn) {    // 求 n 含有 i 因子的数量 
                nn /= i;
                nums2 += nn; 
            }  
            if (nums2 < nums1) { 
                // 少于m中所含i的数量，则m肯定无法整除n! 
                ok = false;
                break; 
            }   
            j = (int)sqrt(1.0*m);   
            // 调整新的素因子前进范围 
        }    
    } 
    if (!ok || m > n || m == 0) return false; 
    else return true; 
}      
 
void Factorial::Output(int n) const {
    if (n > MAXN) {
        cout << "Super Pig, your input is so large, cannot Calculate. Sorry!\n";
        return;
    }     
    int i, len = 8;
    cout << setw(4) << n << "! = ";              
    // 格式控制输出 
    for (i = data[n][0]; i >= 1; i--) {
        cout << data[n][i];
        if (++len == 58) {                               
            // 实际每输出50个字符就换行 
            len = 8;
            cout << "\n        ";
        }    
    }    
    if (len != 8) cout << endl;
}    
 
int main() {
    int n, m, i;
    Factorial f;
    while (cin >> n) {
        f.Calculate(n);
        f.Output(n);
        cout << "该阶乘末尾第一个非0数字是: " << f.FirstNonZero(n) << endl; 
        cout << "该阶乘总共拥有数字0的个数：" << f.CountZeros(n) << endl;
        cout << "该阶乘的左边的第2位数字是：" << f.SecondNum(n) << endl; 
        cin >> m;
        if (f.CanDivide(m, n)) cout << m << " 可以整除 " << n << "!\n";
        else cout << m << " 不能整除 " << n << "!\n";  
    }
    return 0;    
}
 
//第2部分第(5)个算法的单独实现
#include<stdio.h>
int A[10] = {1,1,2,6,24,120,720,5040,40320,362880};
int ans[1024];
int sum;
 
void insert(int* a, int x) { 
    // 插入排序，插成有序表
    int i, j;
    for (i = 1; i <= a[0]; i++)
        if (a[i] >= x) break;
    if (i <= a[0] && a[i] == x) return;
    // 如果相等，则不用插入 
    if (i > a[0]) {
        a[++a[0]] = x;
    }     
    else {
        for (j = a[0]++; j >= i; j--)
            ans[j+1] = ans[j]; 
        ans[i] = x; 
    }     
    if (a[0] == 1023) printf(" Array Overflow!\n"); 
} 
    
void search(int n){
    for (int i = n; i <= 9; i++){
        sum += A[i]; if (sum > 1) insert(ans, sum); 
        if (i < 9) search(i+1);
        sum -= A[i]; 
    }     
}  
 
int main(){
    int n,i;
    ans[0] = 1; ans[1] = 1;                                     
    //初始化ans数组，ans[0]表示表长
    search(0); 
    //printf("len = %d\n", ans[0]); 
    while(1){
        scanf("%d",&n);
        if(n < 0)break;
        if(n > 409114){ printf("NO\n");continue;}
        
        for (i = 1; i <= ans[0]; i++)
            if (ans[i] == n) {printf("YES\n"); break;}
            else if (ans[i] > n) {printf("NO\n"); break;} 
    }  
    return 0;   
}
