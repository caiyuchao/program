/**     ������  :AddLimitAmt
 *      ���ܸ���:�����˻����ģ��
 *      ������������������
 *                ϵͳ��ʶ systemid (���1 С��2)
 *                ���׽��
 *      ����ֵ  :0 �ɹ������� ʧ��
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
        printf( "�����α�ʧ��" );
        EXEC SQL rollback work;
        return 1;
    }

    EXEC SQL open sqlcur;
    EXEC SQL fetch sqlcur INTO :stArStdBkacct;
    if( SQLCODE )
    {
        printf( "fetch�α�ʧ��" );
        EXEC SQL rollback work;
        EXEC SQL close sqlcur;
        return 2;
    }


    EXEC SQL select * INTO :stArStdBkacct from AR_STD_BKACCT where Chkinstgpty = :cChkinstgpty;
    if(!SQLOK)
    {
        if(SQLNOTFOUND)
        {
            ErrLog("���ݿ����޴˼�¼\n");
            return FAIL;
        }
        else
        {
            ErrLog("��ȡ���ݿ��¼ʧ��\n");
            return FAIL;
        }
    }



    /* ��ȡ������Ϣ */
    EXEC SQL select * INTO :stArCtlChannel from ar_ctl_channel where channel = :stArStdBkacct.channel;
    if(!SQLOK)
    {
        if(SQLNOTFOUND)
        {
            ErrLog("���ݿ����޴˼�¼\n");
            return FAIL;
        }
        else
        {
            ErrLog("��ȡ���ݿ��¼ʧ��\n");
            return FAIL;
        }
    }

    if (stArCtlChannel.clearmode[0] == '2')  /* 2-    ������ */
    {
        return 0;
    }
    else if( stArCtlChannel.clearmode[0] != '0' &&  stArCtlChannel.clearmode[0] != '1' )
    {
        ErrLog("��������ģʽ����\n");
        return FAIL;
    }


    if(stArCtlChannel.accountmode[0] == '1')
    /* ʵʱ���� */
    {
        /* ������ */
    }
    else if (stArCtlChannel.accountmode[0] == '2')
    /* ���ܼ��� */
    {
        /* continue */
    }
    else if (stArCtlChannel.accountmode[0] == '3') 
    /* �޹� */
    {
        return 0;
    }
    else
    {
        ErrLog("����ģʽ����\n");
        return FAIL;
    }


    /* ���ܼ��� */
    if(stArCtlChannel.limitflag[0] == '1') 
    /* 1-    ��ȹ��� */
    {
        /* ��ȡͷ��CCBS��� */            
        /* �������ͬ�� */
        /* ���Ӷ�ȿ��Ʊ���뷢���� */
        if(stArStdBkacct.systemid == '1')    /*��� */
        {
            stArStdBkacct.hx_deamt += amt;
            EXEC SQL 
            update tmp_test 
               set hx_deamt = :stArStdBkacct.hx_deamt 
             where current of sqlcur;
        }
        else if(stArStdBkacct.systemid == '2') /*С�� */
        {
            stArStdBkacct.bx_deamt += amt;
            EXEC SQL 
            update tmp_test 
               set bx_deamt = :stArStdBkacct.bx_deamt 
             where current of sqlcur;
        }
        else
        {
            ErrLog("ϵͳ��ʶ����\n");
            return FAIL;
        }

        if( SQLCODE )
        {
            printf( "�޸�ʧ��" );
            EXEC SQL rollback work;
            EXEC SQL close sqlcur;
            return 3;
        }
        EXEC SQL commit work;
        EXEC SQL close sqlcur;
        reutrn 0;

    }
    else if(stArCtlChannel.limitflag[0] == '2') 
    /* 2-�޹� */
    {
        return 0;
    }
    else
    {
        ErrLog("��ȿ��Ʊ�־����\n");
        return FAIL;
    }

    return SUCCESS;
}
