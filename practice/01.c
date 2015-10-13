
// not good
for(theElementIndex = 0; theElementIndex < numberOfElements; theElementIndex++)
	elementArray[theElementIndex] = theElementIndex;

for (i = 0; i < nelems; i++)
	elem[i] = i;


//wrong
str[i++] = str[i++] = ' ';
//should
str[i++] = ' ';
str[i++] = ' ';


//wrong
array[i++] = i;

//wrong
scanf("%d %d", &yr, &profit[yr]);
//should
scanf("%d", &yr);
scanf("%d", &profit[yr]);

//1-4
if( !(c == 'y' || c == 'Y'))
	return;
//should
if( c != 'y' && c != 'Y')
	return;


length = (length < BUFSIZE) ? length : BUFSIZE;
//should
if(lenth > BUFSIZE)
	length = BUFSIZE;

flag = flag ? 0 : 1;
//should
flag = !flag;

quote = (*line == '"') ? 1 : 0;
if (val & 1)
	bit = 1;
else
	bit = 0;
//should
quote = (*line == '"');
bit = val & 1;


//1-5
int read(int *ip){
	scanf("&d", ip);
	return *ip;
}
...
insert(&graph[vert], read(&val), read(&ch));
//must
read(&val);
read(&ch);
insert(&graph[vert], val, ch);

//1-6
n = 1;
printf("%d %d\n", n++, n++);
//must
n = 1;
printf("%d ", n++);
printf("%d\n", n++);


if (argc == 3)
	if ((fin = fopen(argv[1], "r")) != NULL)
		if ((fout = fopen(argv[2], "w")) != NULL) {
			while ((c = getc(fin)) != EOF)
				putc(c, fout);
			fclose(fin); fclose(fout);
		}else
			printf("Can't open output file %s\n", argv[2]);
	else
		printf("Can't open input file %s\n", argv[1]);
else
	printf("Usage: cp inputfile outputfile\n");
//should
if (argc != 3)
	printf("Usage: cp inputfile outputfile\n");
else if ((fin = fopen(argv[1], "r")) != NULL)
	printf("Can't open input file %s\n", argv[1]);
else if ((fout = fopen(argv[2], "w")) != NULL) {
	printf("Can't open output file %s\n", argv[2]);
}else{
	while ((c = getc(fin)) != EOF)
		putc(c, fout);
	fclose(fin);
	fclose(fout);
}

//1-7-1
if (istty(stdin));
else if (istty(stdout)) ;
	else if (istty(stderr)) ;
		else return (0);
//should
if (!istty(stdin) && !istty(stdout) & !istty(stderr)) 
	return (0);

//1-7-2
if (retval != SUCCESS)
{
	return (retval);
}
/* All went well! */
return SUCCESS;
//should
return retval;

//1-7-2
for (k = 0; k++ < 5; x += dx)
	scanf("%lf", &dx);
//should
for (k = 0; k < 5; k++){
	scanf("%lf", &dx);
	x += dx;
}

//1-8
int count = 0;
while (count < total){
	count++;
	if (this.getName(count) == nametable.userName()){
		return (true);
	}
}
//must, java? no...
for(int i = 0; i < total; i++){
	if(strcmp(this.getName(i), nametable.userName()) == 0)
		return true;
}

//1-9
#define ISDIGIT(c) ((c >= '0') && (c <= '9')) ? 1 : 0
//should
#define ISDIGIT(c) (((c) >= '0') && ((c) <= '9')) ? 1 : 0

str = 0;
name[i] = 0;
x = 0;
//should
str = NULL;
name[i] = '\0';
x = 0.0;


