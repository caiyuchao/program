/**     函数名  :AddLimitAmt
 *      功能概述:增加账户余额模块
 *      输入参数：核算机构号
 *                系统标识 systemid (大额1 小额2)
 *                交易金额
 *      返回值  :0 成功，其他 失败
 *
**/
int AddLimitAmt(char  * BranchId,char  systemid ,double  amt)
{
    int iRet;
    char cStr[8];
    char cStr1[3];
    char cStr2[5];

    EXEC SQL BEGIN DECLARE SECTION;
        char    cChkinstgpty[14+1];
        char    cAcct3110no[32+1];
        AR_STD_BKACCT    stArStdBkacct;
        AR_CTL_CHANNEL    stArCtlChannel;
    EXEC SQL END   DECLARE SECTION;

    memset (stArStdBkacct,  0x00, sizeof(stArStdBkacct)); 
    memset (stArCtlChannel, 0x00, sizeof(stArCtlChannel)); 
    memset (cChkinstgpty, 0x00, sizeof(cChkinstgpty)); 

    strcpy(cChkinstgpty,BranchId);

    EXEC SQL DECLARE sqlcur CURSOR FOR 
              select * 
                from AR_STD_BKACCT 
               where Chkinstgpty = :cChkinstgpty 
          for update wait 5;
    if( SQLCODE )
    {
        printf( "声明游标失败" );
        EXEC SQL rollback work;
        return 1;
    }

    EXEC SQL open sqlcur;
    EXEC SQL fetch sqlcur INTO :stArStdBkacct;
    if( SQLCODE )
    {
        printf( "fetch游标失败" );
        EXEC SQL rollback work;
        EXEC SQL close sqlcur;
        return 2;
    }


    EXEC SQL select * INTO :stArStdBkacct from AR_STD_BKACCT where Chkinstgpty = :cChkinstgpty;
    if(!SQLOK)
    {
        if(SQLNOTFOUND)
        {
            ErrLog("数据库中无此记录\n");
            return FAIL;
        }
        else
        {
            ErrLog("读取数据库记录失败\n");
            return FAIL;
        }
    }



    /* 获取渠道信息 */
    EXEC SQL select * INTO :stArCtlChannel from ar_ctl_channel where channel = :stArStdBkacct.channel;
    if(!SQLOK)
    {
        if(SQLNOTFOUND)
        {
            ErrLog("数据库中无此记录\n");
            return FAIL;
        }
        else
        {
            ErrLog("读取数据库记录失败\n");
            return FAIL;
        }
    }

    if (stArCtlChannel.clearmode[0] == '2')  /* 2-    无清算 */
    {
        return 0;
    }
    else if( stArCtlChannel.clearmode[0] != '0' &&  stArCtlChannel.clearmode[0] != '1' )
    {
        ErrLog("渠道清算模式错误\n");
        return FAIL;
    }


    if(stArCtlChannel.accountmode[0] == '1')
    /* 实时记账 */
    {
        /* 待完善 */
    }
    else if (stArCtlChannel.accountmode[0] == '2')
    /* 汇总记账 */
    {
        /* continue */
    }
    else if (stArCtlChannel.accountmode[0] == '3') 
    /* 无关 */
    {
        return 0;
    }
    else
    {
        ErrLog("记账模式错误\n");
        return FAIL;
    }


    /* 汇总记账 */
    if(stArCtlChannel.limitflag[0] == '1') 
    /* 1-    额度管理 */
    {
        /* 获取头寸CCBS余额 */            
        /* 本地余额同步 */
        /* 增加额度控制表汇入发生额 */
        if(stArStdBkacct.systemid == '1')    /*大额 */
        {
            stArStdBkacct.hx_deamt += amt;
            EXEC SQL 
            update tmp_test 
               set hx_deamt = :stArStdBkacct.hx_deamt 
             where current of sqlcur;
        }
        else if(stArStdBkacct.systemid == '2') /*小额 */
        {
            stArStdBkacct.bx_deamt += amt;
            EXEC SQL 
            update tmp_test 
               set bx_deamt = :stArStdBkacct.bx_deamt 
             where current of sqlcur;
        }
        else
        {
            ErrLog("系统标识错误\n");
            return FAIL;
        }

        if( SQLCODE )
        {
            printf( "修改失败" );
            EXEC SQL rollback work;
            EXEC SQL close sqlcur;
            return 3;
        }
        EXEC SQL commit work;
        EXEC SQL close sqlcur;
        reutrn 0;

    }
    else if(stArCtlChannel.limitflag[0] == '2') 
    /* 2-无关 */
    {
        return 0;
    }
    else
    {
        ErrLog("额度控制标志错误\n");
        return FAIL;
    }

    return SUCCESS;
}
