#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int clear = 0;
#define PAGESIZE 4096
typedef unsigned char byte;

typedef struct chunkhead
{
    unsigned int size;
    unsigned int info;
    unsigned char *next, *prev;
}chunkhead;

unsigned char *mymalloc(unsigned int size);
void myfree(unsigned char *address);
void analyze();
chunkhead *head = NULL;//(chunkhead *) myheap;
chunkhead* get_last_chunk();


int main()
{
    
    byte* a[100];
    analyze();
    for(int i=0; i<100; i++)
    {
        a[i] = mymalloc(1000);
    }
    for(int i=0; i<90; i++)
    {
        myfree(a[i]);
    }
    analyze();
    myfree(a[95]);
    a[95] = mymalloc(1000);
    analyze();
    for(int i=90; i<100; i++)
    {
        myfree(a[i]);
    }
    analyze();
    

    return 0;
}

unsigned char *mymalloc(unsigned int size)
{
    unsigned char *ret;
    size += sizeof(chunkhead);
    size -= (size%PAGESIZE);
    size += PAGESIZE;
    
    if(!head)
    {
        void *ptr;
        ptr = sbrk(size);
        head = (chunkhead*)ptr;
        head->info = 1;
        head->next = 0;
        head->prev = 0;
        head->size = size;
        return (unsigned char*)head + sizeof(chunkhead);
    }
    if(head->next == 0)
    {
        if(head->info == 1)
        {
            void *ptr;
            ptr = sbrk(size);
            chunkhead * new = (chunkhead*)ptr;
            new->info = 1;
            new->next = 0;
            new->prev = (unsigned char*)head;
            new->size = size;

            head->next = (unsigned char*)new;
            return (unsigned char*)new + sizeof(chunkhead);
        }
    }

    chunkhead *chunk = head;
    int sift = 0;
    int onchunk = 0;
    int bestChunk = -1;
    int bestSize = -1;

    while(sift == 0)
    {
        if(chunk->info == 0 && size <= chunk->size)
        {
            if(bestSize == -1)
            {
                bestChunk = onchunk;
                bestSize = chunk->size;
            }
            else if(bestSize > -1 && chunk->size < bestSize)
            {
                bestChunk = onchunk;
                bestSize = chunk->size;
            }
        }
        if(chunk->next == 0)
        {
            sift = 1;
        }
        else{
            chunk = (chunkhead *)(chunk->next);
            onchunk += 1;
        }
    }

    if(bestChunk == -1)
    {
        void *ptr;
        ptr = sbrk(size);
        chunkhead * new = (chunkhead*)ptr;
        new->info = 1;
        new->next = 0;
        new->prev = (unsigned char*)chunk;
        new->size = size;

        chunk->next = (unsigned char*)new;
        return (unsigned char*)new + sizeof(chunkhead);
    }

    chunk = head;
    onchunk = 0;

    while(onchunk<bestChunk)
    {
        chunk = (chunkhead *)(chunk->next);
        onchunk += 1;
    }

    if(chunk->size == size)
    {
        chunk->info = 1;
        return (unsigned char *)chunk + sizeof(chunkhead);
    }
    
    chunkhead *new = chunk+size;
    new->info = 0;
    new->prev = (unsigned char *)chunk;
    new->next = chunk->next;
    new->size = (chunk->size) - size;
    
    if(new->next != 0)
    {
        if(((chunkhead*)(new->next))->info == 0)
        {
            new->next = ((chunkhead*)(new->next))->next;
            new->size = new->size + ((chunkhead*)(new->next))->size;
        }
    }

    chunk->info = 1;
    chunk->next = (unsigned char *)new;
    chunk->size = size;
    return (unsigned char *)chunk + sizeof(chunkhead);
}

void myfree(unsigned char *address)
{
    unsigned char *chunkaddy = address - sizeof(chunkhead);
    chunkhead *chunk = (chunkhead *)chunkaddy;
    
    if(chunk->info == 1)
    {
        if(chunk->next == 0)
        {
            int secondEmpt = 0;
            if(chunk->prev != 0)
            {
                if(((chunkhead *)(chunk->prev))->info == 0)
                {
                    ((chunkhead *)(chunk->prev))->info = 0;
                    if(((chunkhead *)(chunk->prev))->prev != 0)
                    {
                        ((chunkhead *)((chunkhead *)(chunk->prev))->prev)->next = 0;
                    }
                    else
                    {
                        head = NULL;
                    }
                    secondEmpt = ((chunkhead *)(chunk->prev))->size;
                }
                ((chunkhead *)(chunk->prev))->next = 0;
            }
            else
            {
                head = NULL;
            }
            void *ptr;
            signed int ggg =  0 - (chunk->size)-secondEmpt;
            ptr = sbrk(ggg);
        }
        else
        {
            chunk->info = 0;
            if(chunk->prev != 0)
            {
                if(((chunkhead *)(chunk->prev))->info == 0)
                {
                    ((chunkhead *)(chunk->prev))->next = chunk->next;
                    ((chunkhead *)(chunk->prev))->size = (chunk->size + ((chunkhead *)(chunk->prev))->size);
                    ((chunkhead *)(chunk->next))->prev = chunk->prev;
                }
            }
        }
        return;
    }
    printf("Memory Already Free  \n");
}

void analyze()
{
    printf("\n--------------------------------------------------------------------------------------------\n");
    if(!head)
    {
        printf("No Heap\n");
        printf("program break on address: %x\n", sbrk(0));
        return;
    }
    chunkhead* ch = (chunkhead*)head;
    for (int no = 0; ch; ch = (chunkhead*)ch->next, no++)
    {
        printf("%d | current addr: %x |", no, ch);
        printf("size: %d | ", ch->size);
        printf("info: %d | ", ch->info);
        printf("next: %x | ", ch->next);
        printf("prev: %x    \n", ch->prev);
    }
    printf("program break on address: %x\n", sbrk(0));
    return;
}

chunkhead* get_last_chunk()
{
    if(!head) //void *head == NULL
    {
        return NULL;
    }
    chunkhead* ch = (chunkhead*)head;
    for(; ch->next; ch = (chunkhead*)ch->next);
    return ch;
}