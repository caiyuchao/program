#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oci.h"

static OCIEnv           *p_env;
static OCIError         *p_err;
static OCISvcCtx        *p_svc;
static OCIStmt          *p_sql;
static OCIDefine        *p_dfn    = (OCIDefine *) 0;
static OCIBind          *p_bnd    = (OCIBind *) 0;

int main()
{
  int             p_bvi;
  char            p_sli[1024];
  char            p_sli2[1024];
  int             rc;
  char            errbuf[100];
  int             errcode;
  char connstr[] = "(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=(PROTOCOL=TCP)(HOST=128.96.96.121)(PORT=1521)))(CONNECT_DATA=(SERVICE_NAME=hvps)))";

  rc = OCIInitialize((ub4) OCI_DEFAULT, (dvoid *)0, 
      (dvoid * (*)(dvoid *, size_t)) 0,
      (dvoid * (*)(dvoid *, dvoid *, size_t))0,
      (void (*)(dvoid *, dvoid *)) 0 );

  
  rc = OCIEnvInit( (OCIEnv **) &p_env, OCI_DEFAULT, (size_t) 0, (dvoid **) 0 );

  
  rc = OCIHandleAlloc( (dvoid *) p_env, (dvoid **) &p_err, OCI_HTYPE_ERROR,
      (size_t) 0, (dvoid **) 0);
  rc = OCIHandleAlloc( (dvoid *) p_env, (dvoid **) &p_svc, OCI_HTYPE_SVCCTX,
      (size_t) 0, (dvoid **) 0);

  
  rc = OCILogon(p_env, p_err, &p_svc, "arps", 4, "arps", 4, connstr, strlen(connstr));
  if (rc != 0) {
    OCIErrorGet((dvoid *)p_err, (ub4) 1, (text *) NULL, &errcode, errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
    printf("Error - %.*s\n", 512, errbuf);
    exit(8);
  }
  else
    printf("connection success!\n");

  
  rc = OCIHandleAlloc( (dvoid *) p_env, (dvoid **) &p_sql,
      OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0);
 

  char sql[] = "select 'hello world' as A from dual";
  rc = OCIStmtPrepare(p_sql, p_err, sql,
      (ub4) strlen(sql) , (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);

  
  p_bvi = 10;    
  
  rc = OCIDefineByPos(p_sql, &p_dfn, p_err, 1, p_sli, 1024, SQLT_STR, (dvoid *) 0, (ub2 *)0,
      (ub2 *)0, OCI_DEFAULT);

  rc = OCIDefineByPos(p_sql, &p_dfn, p_err, 2, p_sli2, 1024, SQLT_STR, (dvoid *) 0, (ub2 *)0,
      (ub2 *)0, OCI_DEFAULT);


  
  rc = OCIStmtExecute(p_svc, p_sql, p_err, (ub4) 1, (ub4) 0,
      (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);

  ub4         ColumnCount;

  

  OCIAttrGet((dvoid*)p_sql, (ub4)OCI_HTYPE_STMT, (dvoid*)&ColumnCount, (ub4 *) 0, (ub4)OCI_ATTR_PARAM_COUNT, p_err);
  printf(" Coulumcount=%d\n",ColumnCount);

  ub4   pos;  
  ub2   dtype;  
  ub2   dsize;  
  text   *name;  
  ub4   name_length;  
  OCIParam   *pparam;
  char realname[1024];

  pos = 1;

  while   (OCIParamGet(p_sql, OCI_HTYPE_STMT, p_err, (dvoid *)&pparam, pos)   ==   OCI_SUCCESS)  
  {  
    OCIAttrGet((dvoid   *)pparam,   OCI_DTYPE_PARAM,   (dvoid   *)&dtype,   0,   OCI_ATTR_DATA_TYPE,   p_err);  
    OCIAttrGet((dvoid   *)pparam,   OCI_DTYPE_PARAM,   (dvoid   *)&dsize,   0,   OCI_ATTR_DATA_SIZE,   p_err);  
    OCIAttrGet((dvoid   *)pparam,   OCI_DTYPE_PARAM,   (dvoid   *)&name,   &name_length,   OCI_ATTR_NAME,   p_err);  
    strncpy(realname, name, name_length);
    realname[name_length] = 0;
    printf("%02d >column   =   '%s',   type   =   %d,   size   =   %d.\n",   pos,   realname,   dtype,   dsize);

    pos++;
  }  


  rc = OCILogoff(p_svc, p_err);                          
  rc = OCIHandleFree((dvoid *) p_sql, OCI_HTYPE_STMT);   
  rc = OCIHandleFree((dvoid *) p_svc, OCI_HTYPE_SVCCTX);
  rc = OCIHandleFree((dvoid *) p_err, OCI_HTYPE_ERROR);

  return;
}

