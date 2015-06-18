int Change_Value = 0;

int Sum_it (int, int);
void error_with_code (void);

 /******************************************/
 /*                                           */
 /* main () - main routine                    */
 /*                                           */
/******************************************/

int main (void)
{
   int Arg_1 = 20, Arg_2 = 40;
   int i;


    for (i = 0; i < 10; i++) {
       if (Arg_1 > Arg_2)
            Arg_2 = Sum_it (Arg_1, Arg_2);
       else
           Arg_1 = Sum_it (Arg_1, Arg_2);
    }

    error_with_code ();

}   /* End of main() */


/**********************************************************/
/*                                                        */
/* Sum_it() - This routine adds two numbers and returns the
result */
/*                                                        */
/**********************************************************/
int Sum_it (int a, int b)

{
    return a+b;

} /* End of Sum_it */



/**********************************************************/
/*                                                        */
/* error_with_code() - This routine has an intended bug.  */
/******************************************************** */
void error_with_code(void)
{
   int divide_value;
   int result;

   divide_value = Change_Value;
   result = 10 / divide_value;
   return ;

} /* End of error_with_code() */
