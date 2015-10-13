//2.1 searching
char *flab[] = {
	"actually",
	"just",
	"quite",
	"really",
	NULL
};

/* lookup: sequential search for word in array */
int lookup(char *word, char *array[])
{
	int i;
	for (i = 0; array[i] != NULL; i++)
		if (strcmp(word, array[i]) == 0)
			return i;
	return -1;
}


//-----------------
typedef struct Nameval Nameval;
struct Nameval {
	char *name;
	int  value;
};

/* HTML characters, e.g. AElig is ligature of A and E. */
/* Values are Unicode/ISO10656 encoding. */
Nameval htmlchar[] = {
	"AElig",  0x00c6,
	"Aacute", 0x00c1,
	"Acirc",  0x00c2,
	/* ... */
	"zeta",   0x03b6,
};

#define NELEMS(array) (sizeof(array)/sizeof(array[0]))
printf("The HTML table hash %d words\n", NELEMS(htmlchars));
/* lookup: binary search for name in tab; return index */
int lookup(char *name, Nameval tab[], int ntab)
{
	int low, high, mid, cmp;
	low = 0;
	high = ntab -1;
	while(low <= high){
		mid = (low + hight) / 2;
		cmp = strcmp(name, tab[mid].name);
		if(cmp <0)
			high = mid - 1;
		else if (cmp > 0)
			low = mid + 1;
		else /* found match */
			return mid;
	}
	return -1; /* no match */
}

half = lookup("frac12", htmlchar, NELEMS(htmlchars));

//2.2 sort
//---
/* quicksort: sort v[0]..v[n-1] into increasing order */
void quicksort(int v[], int n)
{
	int i, last;
	if (n <= 1) /* nothing to do */
		return;
	swap(v, 0, rand()%n);     /* move pivot elem to v[0] */
	last = 0;
	for (i = 1; i < n; i++)   /* partition */
		if (v[i] < v[0])
			swap(v, ++last, i);
	swap(v, 0, last);              /* restore pivot */
	quicksort(v, last);            /* recursively sort */
	quicksort(v+last+1, n-last-1); /* echo part */
}

/* swap: interchange v[i] and v[j] */
void swap(int v[], int i, int j)
{
	int temp;
	temp = v[i];
	v[i] = v[j];
	v[j] = temp;
}


//2.3 libraries
//qsort
/* scmp: string compare of *p1 and *p2 */
int scmp(const void *p1, const void *p2)
{
	char *v1, *v2;

	v1 = *(char **)p1;
	v2 = *(char **)p2;
	return strcmp(v1, v2);
}
char *str[N];
qsort(str, N, sizeof(str[0]), scmp);

/* icmp: integer compare of *p1 and *p2 */
int icmp(const void *p1, const void *p2)
{
	int v1, v2;
	
	v1 = *(int *)p1;
	v2 = *(int *)p2;
	if (v1 < v2)
		return -1;
	else if(v1 == v2)
		return 0;
	else
		return 1;
}
int i[N];
qsort(i, N, sizeof(i[0]), icmp);


//bsearch
/* lookup: use bsearch to find name in tab, return index */
int lookup(char *name, Nameval tab[], int ntab)
{
	Nameval key, *np;

	key.name = name;
	key.value = 0; /* unused; anything will do */
	np = (Nameval *) bsearch(&key, tab, ntab,
			sizeof(tab[0]), nvcmp);
	if(np == NULL)
		return -1;
	else
		return (np - tab)/ sizeof(tab[0]);
}

/* nvcmp: compare tow Nameval names */
int vncmp(const void *va, const void *vb)
{
	const Nameval *a, *b;

	a = (Nameval *) va;
	b = (Nameval *) vb;
	return strcmp(a->name, b->name);
}

// exercise 2-1
// iteratively qsort
typedef long type;                                         /* array type */
#define MAX 64            /* stack size for max 2^(64/2) array elements  */

void quicksort_iterative(type n cvxarray[], unsigned len) {
   unsigned left = 0, stack[MAX], pos = 0, seed = rand();
   for ( ; ; ) {                                           /* outer loop */
      for (; left+1 < len; len++) {                /* sort left to len-1 */
         if (pos == MAX) len = stack[pos = 0];  /* stack overflow, reset */
         type pivot = array[left+seed%(len-left)];  /* pick random pivot */
         seed = seed*69069+1;                /* next pseudorandom number */
         stack[pos++] = len;                    /* sort right part later */
         for (unsigned right = left-1; ; ) { /* inner loop: partitioning */
            while (array[++right] < pivot);  /* look for greater element */
            while (pivot < array[--len]);    /* look for smaller element */
            if (right >= len) break;           /* partition point found? */
            type temp = array[right];
            array[right] = array[len];                  /* the only swap */
            array[len] = temp;
         }                            /* partitioned, continue left part */
      }
      if (pos == 0) break;                               /* stack empty? */
      left = len;                             /* left to right is sorted */
      len = stack[--pos];                      /* get next range to sort */
   } 
}


// 2.4 a java quicksort
interface Cmp {
	int cmp(Object x, Object y);
}

// Icmp: Integer comparison
class Icmp implements Cmp{
	public int cmp(Object o1, Object o2)
	{
		int i1 = ((Integer) o1).intValue();
		int i2 = ((Integer) o2).intValue();
		if (i1 < i2)
			return -1;
		else if (i1 == i2)
			return 0;
		else
			return 1;
	}
}
// Scmp: String comparison
class Scmp implements Cmp{
	public int cmp(Object o1, Object o2)
	{
		String s1 = (String) o1;
		String s2 = (String) o2;
		return s1.compareTo(s2);
	}
}
// Quicksort.sort: quicksort v[left] . .v[right]
static void sort(Object[] v, int left , int right, Cmp cmp)
{
	int i, last;
	if ( left >= right) // nothing t o do
		return;
	swap(v, l e f t , rand(1eft. right)) ; // move pivot elem
	last = left; // to v[left]
	for (i= left + l; i <= right; i++) // partition
		if (cmp.cmp(v[i], v[left]) < 0)
			swap(v, ++last, i);
	swap(v, left , last); // restore pivot elem
	sort(v, left , last-1, cmp); // recursively sort
	sort(v, last + l , right , cmp) ; // each part
}
// Quicksort.swap: swap v [ i ] and v [ j ]
static void swap(Object[] v, int i, int j)
{
	Object temp;
	temp = v[i];
	v[i] = v[j];
	v[j] = temp;
}

static Random rgen = new Random();
// Quicksort.rand: return random integer in [left, right]
static int rand(int left, int right)
{
	return 1eft + Math.abs(rgen.nextInt())%(right-left+l);
}

String[] sarr = new String[n];
// fill n elements of sarr . . .
Quicksort.sort(sarr, 0, sarr.length-1, new Scmp());



// 2.6 Growing Arrays
typedef struct Nameval Nameval;
struct Nameval {
	char *name;
	int  value;
};

struct NVtab{
	int nval;          /* current number of values */
	int max;           /* allocated number of values */
	Nameval *nameval;  /* array of name-value pairs */
} nvtab;

enum { NVINIT = 1, NVGROW = 2};

/* addname: add new name and value to nvtab */
int addname(Nameval newname)
{
	Nameval *nvp;
	if (nvtab.nameval == NULL) { /* first time */
		nvtab.nameval = (Nameval *)malloc(NVINIT * sizeof(Nameval));
		if(nvtab.nameval == NULL)
			return -1;
		nvtab.max = NVINIT;
		nvtab.nval = 0;
	}else if (nvtab.nval >= nvtab.max) { /* grow */
		nvp = (Nameval *) realloc(nvtab.nameval,
				(NVGROW*nvtab.max) * sizeof(Nameval));
		if(nvp == NULL)
			return -1;
		nvtab.max *= NVGROW;
		nvtab.nameval = nvp;
	}
	nvtab.nameval[nvtab.nval] = newname;
	return nvtab.nval++;
}

/* delname: remove first matching nameval from nvtab */
int delname(char *name)
{
	int i;

	for (i = 0; i < nvtab.nval; i++)
		if (strcmp(nvtab.nameval[i].name, name) == 0){
			memmove(nvtab.nameval + i, nvtab.nameval+i+1,
					(nvtab.nval-(i+1))*sizeof(Nameval));
			nvtab.nval--;
			return 1;
		}
	return 0;
}


//2.7 lists
//2.8 trees
//2.9 hash tables
