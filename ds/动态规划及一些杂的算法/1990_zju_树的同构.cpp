#include<stdio.h>
#include<string.h>
int trees[1505][1505];
int lens[1505];
int ds[1505];

int treet[1505][1505];
int lent[1505];
int dt[1505];

char s[3005], t[3005];
char ps[3005], pt[3005];
int flag[1505];
int tops,topt;
int l;

void BuildTrees( int u, char *s )
{
     if( s[l] == 0 )return;
    
     while( s[l] && s[l] == '0' )
     {
         trees[u][lens[u]++] = ++tops;
         l++;
         ds[u]++;
         BuildTrees( tops, s );
         if( s[l] == 0 )return;
     }
     if( s[l] == 0 )return;
     l++;
     return;
}

void BuildTreet( int u, char t[] )
{
     if( t[l] == 0 )return;
     while( t[l] && t[l] == '0' )
     {
         treet[u][lent[u]++] = ++topt;
         l++;
         dt[u]++;
         BuildTreet( topt, t );
         if( t[l] == 0 )return;
     }
     if( t[l] == 0 )return;
     l++;
     return;
}

int Copx( int u,int v )
{
     int i, t;
     for( i = 0; i < lens[u]; i++ )
     {
         if( ds[trees[u][i]] < ds[trees[v][i]] )return -1;
         else if( ds[trees[u][i]] > ds[trees[v][i]] )return 1;
         else
         {
             t = Copx( trees[u][i], trees[v][i] );
             if( t != 0 )return t;
         }
     }
     return 0;
} 

void ReBuilds( int u )
{
     int i, j, t;
     for( i = 0; i < lens[u]; i++ )
     {
         ReBuilds( trees[u][i] );
     }
     for( j = 0; j < lens[u]-1; j++ )
     {
         for( i = j+1; i < lens[u]; i++ )
         {
             if( ds[trees[u][i]] < ds[trees[u][j]] )
             {
                 t = trees[u][i];
                 trees[u][i] = trees[u][j];
                 trees[u][j] = t; 
             }
             else if( ds[trees[u][i]] == ds[trees[u][j]] )
             {
                 if( Copx( trees[u][i], trees[u][j] ) < 0 )
                 {
                     t = trees[u][i];
                     trees[u][i] = trees[u][j];
                     trees[u][j] = t;  
                 } 
             }
         }
     }
}

int Copy( int u,int v )
{
     int i, t;
     for( i = 0; i < lent[u]; i++ )
     {
         if( dt[treet[u][i]] < dt[treet[v][i]] )return -1;
         else if( dt[treet[u][i]] > dt[treet[v][i]] )return 1;
         else
         {
             t = Copy( treet[u][i], treet[v][i] );
             if( t != 0 )return t;
         }
     }
     return 0;
} 

void ReBuildt( int u )
{
     int i, j, t;
     for( i = 0; i < lent[u]; i++ )
     {
         ReBuildt( treet[u][i] );
     }
     for( j = 0; j < lent[u]-1; j++ )
     {
         for( i = j+1; i < lent[u]; i++ )
         {
             if( dt[treet[u][i]] < dt[treet[u][j]] )
             {
                 t = treet[u][i];
                 treet[u][i] = treet[u][j];
                 treet[u][j] = t; 
             }
             else if( dt[treet[u][i]] == dt[treet[u][j]] )
             {
                 if( Copy( treet[u][i], treet[u][j] ) < 0 )
                 {
                     t = treet[u][i];
                     treet[u][i] = treet[u][j];
                     treet[u][j] = t;  
                 } 
             }
         }
     }
}

void DFSS( int u )
{
     flag[u] = 1;
     for( int i = 0; i < lens[u]; i++ )
     {
         if( flag[trees[u][i]] == 0 )
         {
             ps[tops++] = '0';
             DFSS( trees[u][i] );
             ps[tops++] = '1';
         }
     }
}

void DFST( int u )
{
     flag[u] = 1;
     for( int i = 0; i < lent[u]; i++ )
     {
         if( flag[treet[u][i]] == 0 )
         {
             pt[topt++] = '0';
             DFST( treet[u][i] );
             pt[topt++] = '1';
         }
     }
}

void GetAns()
{
     tops = 0;
     memset( lens, 0, sizeof( lens ) );
     memset( ds, 0, sizeof( ds ) );
     l = 0;
     BuildTrees( 0, s );
     topt = 0;
     memset( lent, 0, sizeof( lent ) );
     memset( dt, 0, sizeof( dt ) );
     l = 0;
     BuildTreet( 0, t );
    
     ReBuilds( 0 );
     ReBuildt( 0 );
     memset( flag, 0, sizeof( flag ) );
     tops = 0;
     DFSS(0);
     ps[tops] = 0;
     topt = 0;
     memset( flag, 0, sizeof( flag ) );
     DFST(0);
     pt[topt] = 0;
     if( strcmp( ps, pt ) == 0 )printf( "same\n" );
     else printf( "different\n" );
    
}

int main()
{
     int kcase;
     scanf( "%d", &kcase );
     while( kcase-- )
     {
         scanf( "%s %s", s, t );
         GetAns();
     } 
     return 0;
}

 

