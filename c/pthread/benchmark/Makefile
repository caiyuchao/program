.PHONY:affinitytwothread  \
       affinityunlockcacheprocess \
       lockcachemultiprocess \
       lockmultiprocess \
       twothreadprocess \
       uniprocess \
       unlockcachemultiprocess \
       unlockmultiprocess \

all:affinitytwothread affinityunlockcacheprocess lockcachemultiprocess lockmultiprocess twothreadprocess uniprocess unlockcachemultiprocess unlockmultiprocess


affinitytwothread:affinitytwothread.c
	gcc -o affinitytwothread affinitytwothread.c -lpthread

affinityunlockcacheprocess:affinityunlockcacheprocess.c
	gcc -o affinityunlockcacheprocess affinityunlockcacheprocess.c -lpthread

lockcachemultiprocess:lockcachemultiprocess.c
	gcc -o lockcachemultiprocess lockcachemultiprocess.c -lpthread

lockmultiprocess:lockmultiprocess.c
	gcc -o lockmultiprocess lockmultiprocess.c -lpthread

twothreadprocess:twothreadprocess.c
	gcc -o twothreadprocess twothreadprocess.c -lpthread

uniprocess:uniprocess.c
	gcc -o uniprocess uniprocess.c 

unlockcachemultiprocess:unlockcachemultiprocess.c
	gcc -o unlockcachemultiprocess unlockcachemultiprocess.c -lpthread

unlockmultiprocess:unlockmultiprocess.c
	gcc -o unlockmultiprocess unlockmultiprocess.c -lpthread


clean:
	rm -f affinitytwothread affinityunlockcacheprocess lockcachemultiprocess lockmultiprocess twothreadprocess uniprocess unlockcachemultiprocess unlockmultiprocess
