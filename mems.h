/*
All the main functions with respect to the MeMS are inplemented here
read the function discription for more details

NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
you are only allowed to implement the functions
you can also make additional helper functions a you wish

REFER DOCUMENTATION FOR MORE DETAILS ON FUNSTIONS AND THEIR FUNCTIONALITY
*/
// add other headers as required
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

int pagesused = 0;
int count = 0;
bool return_value = false;

//! Defining starting mems virtual address
int custom_virtual_address = 1000;

#ifdef NDEBUG
#define DEBUG(M, ...)
#define PRINT_FREELIST print_freelist
#else
#define DEBUG(M, ...) fprintf(stderr, "[DEBUG] %s:%d: " M "\n", _FILE, __LINE, ##__VA_ARGS_)

#define PRINT_FREELIST
#endif
/*
Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this
macro to make the output of all system same and conduct a fair evaluation.
*/
#define PAGE_SIZE 4096

bool head_checked = false;

enum NodeType
{
    HOLE,
    PROCESS
};

struct freeList_Main_Node

{

    struct freeList_Main_Node *prev;
    int size;
    void *virtualAddress_Start;
    void *physicalAddress_Start;
    void *virtualAddress_End;
    void *physicalAddress_End;
    struct freeList_Main_Node *next;
    struct subList_sub_Node *sub_head; // Sub-list head
    struct subList_sub_Node *sub_tail; // Sub-list tail
};

struct freeList_Main_Node *mainhead = NULL;
struct freeList_Main_Node *maintail = NULL;
struct freeList_Main_Node *head = NULL;

// head is the first node of the main list

struct subList_sub_Node

{
    struct subList_sub_Node *prev_sub;
    void *virtualAddress_subStart;
    void *physicalAddress_subStart; // the adress returned by mmap
    void *virtualAddress_subEnd;
    void *physicalAddress_subEnd;
    struct subList_sub_Node *next_sub;
    enum NodeType type;
    int size;
};

/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_init()
{
    // requesting memory from the system

    head = (struct freeList_Main_Node *)mmap(NULL, sizeof(struct freeList_Main_Node), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (head == MAP_FAILED)
    {
        return_value = false;
        perror("mmap");
        exit(EXIT_FAILURE);

        // mapping failed
    }
    head->prev = NULL;
    // head->size = PAGE_SIZE; // Initial size for the main node
    head->next = NULL;
    head->sub_head = NULL;
    head->sub_tail = NULL;
    maintail = head;
    mainhead = head;

    return_value = true;

    struct freeList_Main_Node *head1 = head;
    while (head1 != NULL)
    {
        DEBUG("\t freelist Size:%d,Head:%p,Prev:%p,Next:%p\t", head1->size,
              head1,
              head1->prev,
              head1->next);
        head1 = head1->next;
    }
    DEBUG("\n");
}

/*
This function will be called at the end of the MeMS system and its main job is to unmap the
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_finish()
{
    struct freeList_Main_Node *traversal_main_node = head;

    while (traversal_main_node != NULL)
    {
        struct freeList_Main_Node *temp_main_node = traversal_main_node;
        struct subList_sub_Node *traversal_sub_node = traversal_main_node->sub_head;

        while (traversal_sub_node != NULL)
        {
            struct subList_sub_Node *temp_sub_node = traversal_sub_node;

            traversal_sub_node = traversal_sub_node->next_sub;
            munmap(temp_sub_node->physicalAddress_subStart, temp_sub_node->size);
            munmap(temp_sub_node, sizeof(struct subList_sub_Node));
        }

        traversal_main_node = traversal_main_node->next;
        munmap(temp_main_node->physicalAddress_Start, temp_main_node->size);
        munmap(temp_main_node, sizeof(struct freeList_Main_Node));
    }
}

/*
Allocates memory of the specified size by reusing a segment from the free list if
a sufficiently large segment is available.

Else, uses the mmap system call to allocate more memory on the heap and updates
the free list accordingly.

Note that while mapping using mmap do not forget to reuse the unused space from mapping
by adding it to the free list.
Parameter: The size of the memory the user program wants
Returns: MeMS Virtual address (that is created by MeMS)
*/
void *mems_malloc(size_t size_asked)
{

    struct freeList_Main_Node *currentNode = head;
    // struct subList_sub_Node *currentsubNode1 = NULL;

    bool no_proper_size_found = true;

    int total_pages_required = (size_asked + PAGE_SIZE - 1) / PAGE_SIZE;

    size_t new_mainNode_size = total_pages_required * PAGE_SIZE;

    struct freeList_Main_Node *new_main_node = NULL;

    if (head == NULL)
    {

        return NULL;
    }

    //! struct freeList_Main_Node *currentNode = head;

    struct subList_sub_Node *newsegment = NULL;
    struct subList_sub_Node *newsegment2 = NULL;

    // while (currentNode != NULL)
    // {
    //     struct subList_sub_Node *subChainNode = currentNode->sub_head;
    // if currentNode is head
    // ek varible daalna ho
    // no of main nodes ka count

    while (currentNode != NULL)
    {
        if (currentNode == head && currentNode->sub_head == NULL && currentNode->sub_tail == NULL && head_checked == false)
        {

            void *addressof_allocatedMemory = mmap(NULL, new_mainNode_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (addressof_allocatedMemory == MAP_FAILED)
            {
                perror("mmap");
                exit(EXIT_FAILURE);
            }
            pagesused += total_pages_required;
            currentNode->size = new_mainNode_size;
            newsegment = (struct subList_sub_Node *)mmap(NULL, sizeof(struct subList_sub_Node), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            currentNode->sub_head = newsegment;
            currentNode->sub_tail = newsegment;
            // Allocate memory for the data

            newsegment->size = size_asked;
            newsegment->physicalAddress_subStart = addressof_allocatedMemory;
            newsegment->physicalAddress_subEnd = newsegment->physicalAddress_subStart + newsegment->size - 1;
            (newsegment->virtualAddress_subStart) = custom_virtual_address;
            newsegment->virtualAddress_subEnd = newsegment->virtualAddress_subStart + (newsegment->physicalAddress_subEnd - newsegment->physicalAddress_subStart);
            currentNode->physicalAddress_Start = addressof_allocatedMemory;
            currentNode->physicalAddress_End = currentNode->physicalAddress_Start + currentNode->size - 1;
            (currentNode->virtualAddress_Start) = custom_virtual_address;
            currentNode->virtualAddress_End = currentNode->virtualAddress_Start + (currentNode->physicalAddress_End - currentNode->physicalAddress_Start);
            newsegment->type = PROCESS;

            newsegment2 = (struct subList_sub_Node *)mmap(NULL, sizeof(struct subList_sub_Node), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            currentNode->sub_tail = newsegment2;
            newsegment2->type = HOLE;
            newsegment2->size = new_mainNode_size - size_asked;
            newsegment2->physicalAddress_subStart = newsegment->physicalAddress_subEnd + 1;
            newsegment2->physicalAddress_subEnd = newsegment2->physicalAddress_subStart + newsegment2->size - 1;
            newsegment2->virtualAddress_subStart = newsegment->virtualAddress_subEnd + 1;
            newsegment2->virtualAddress_subEnd = newsegment2->virtualAddress_subStart + (newsegment2->physicalAddress_subEnd - newsegment2->physicalAddress_subStart);

            newsegment->next_sub = newsegment2;

            newsegment2->prev_sub = newsegment;
            newsegment2->next_sub = NULL;
            currentNode->sub_tail = newsegment2;

            head_checked = true;
            // currentNode = currentNode->next;

            // segment-> size-size_asked =
            no_proper_size_found = false;
            return (void *)(newsegment->virtualAddress_subStart);
        }

        //!<----------------------------------- Sabh  tgeeek hai codeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee----------------------------------------------------->>

        //     currentNode = currentNode->next;
        // }
        else
        {
            count++;
            int currentnodesize = currentNode->size;
            // printf(" Size kya hai?????? %d\n", currentnodesize);

            if (size_asked > currentnodesize)
            {
                // count++;
                // printf("size zyada haiiii aage bado!!\n");
                currentNode = currentNode->next;
            }

            else if (size_asked <= currentnodesize)
            {
                // printf("%d\n", currentNode->sub_head->size);

                //! no proper size found segment pe bhi depend karega ki if space is less then new main node add kariyo
                // printf("Why\n");
                struct subList_sub_Node *currentsubNode1 = currentNode->sub_head;

                // printf("%d", currentsubNode->size);

                while (currentsubNode1 != NULL)
                {

                    // printf("Sub-Node Type: %d, Size: %d\n", currentsubNode1->type, currentsubNode1->size);
                    //  Your other conditions and logic

                    if (currentsubNode1->type == HOLE)
                    {
                        // printf("N\n");
                        if ((currentsubNode1->size) == size_asked)
                        {
                            // printf("Ghus gya\n");
                            currentsubNode1->type = PROCESS;

                            // newsegment = (struct subList_sub_Node *)mmap(NULL, sizeof(struct subList_sub_Node), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

                            // add new subnode
                            //  size of new subnode = size - size_asked
                            //  currentsubnode is just a pointer
                            // currentsubNode=currentsubNode->next_sub;
                            no_proper_size_found = false;

                            return (void *)(currentsubNode1->virtualAddress_subStart);

                            // currentsubNode=currentsubNode->next_sub;

                            // currentsubNode->physicalAddress = addressof_allocatedMemory;
                        }
                        else if ((currentsubNode1->size) > size_asked)
                        {

                            if (currentsubNode1->next_sub == NULL)
                            {
                                //! last subnode ho toh pointer ko shift kar dena

                                currentsubNode1->type = PROCESS;
                                int currentsubNodeOriginalsize = currentsubNode1->size;
                                int lastvirtualaddress = currentsubNode1->virtualAddress_subEnd;
                                int lastphysicaladdress = currentsubNode1->physicalAddress_subEnd;
                                newsegment = (struct subList_sub_Node *)mmap(NULL, sizeof(struct subList_sub_Node), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                                currentsubNode1->next_sub = newsegment;
                                newsegment->prev_sub = currentsubNode1;
                                newsegment->size = currentsubNode1->size - size_asked;

                                currentsubNode1->size = size_asked;
                                currentsubNode1->physicalAddress_subEnd = currentsubNode1->physicalAddress_subStart + currentsubNode1->size - 1;
                                currentsubNode1->virtualAddress_subEnd = currentsubNode1->virtualAddress_subStart + (currentsubNode1->physicalAddress_subEnd - currentsubNode1->physicalAddress_subStart);

                                newsegment->type = HOLE;
                                newsegment->physicalAddress_subStart = currentsubNode1->physicalAddress_subEnd + 1;
                                newsegment->virtualAddress_subStart = currentsubNode1->virtualAddress_subEnd + 1;
                                newsegment->physicalAddress_subEnd = lastphysicaladdress;
                                newsegment->virtualAddress_subEnd = lastvirtualaddress;
                                newsegment->next_sub = NULL;
                                currentNode->sub_tail = newsegment;

                                no_proper_size_found = false;
                                return (void *)(currentsubNode1->virtualAddress_subStart);

                                // printf("Yashu\n");

                                // /newsegment->size=
                            }
                            else if (currentsubNode1->next_sub != NULL)
                            {
                                currentsubNode1->type = PROCESS;
                                int currentsubNodeOriginalsize = currentsubNode1->size;
                                newsegment = (struct subList_sub_Node *)mmap(NULL, sizeof(struct subList_sub_Node), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                                newsegment->next_sub = currentsubNode1->next_sub;
                                currentsubNode1->next_sub = newsegment;
                                newsegment->next_sub->prev_sub = newsegment;
                                newsegment->prev_sub = currentsubNode1;
                                newsegment->size = currentsubNode1->size - size_asked;

                                currentsubNode1->size = size_asked;

                                currentsubNode1->physicalAddress_subEnd = currentsubNode1->physicalAddress_subStart + currentsubNode1->size - 1;
                                currentsubNode1->virtualAddress_subEnd = currentsubNode1->virtualAddress_subStart + (currentsubNode1->physicalAddress_subEnd - currentsubNode1->physicalAddress_subStart);

                                newsegment->type = HOLE;
                                newsegment->physicalAddress_subStart = currentsubNode1->physicalAddress_subEnd + 1;
                                newsegment->virtualAddress_subStart = currentsubNode1->virtualAddress_subEnd + 1;
                                newsegment->physicalAddress_subEnd = newsegment->physicalAddress_subStart + (currentsubNodeOriginalsize - size_asked) - 1;
                                newsegment->virtualAddress_subEnd = newsegment->virtualAddress_subStart + (currentsubNodeOriginalsize - size_asked) - 1;

                                // printf("yashvinder singh ki aisi ki taisi");

                                no_proper_size_found = false;

                                return (void *)(currentsubNode1->virtualAddress_subStart);

                                // newsegment mai add karna
                            }
                        }
                        else
                        {

                            no_proper_size_found = true;
                        }
                    }
                    // printf("Yes");
                    // printf(" ");
                    currentsubNode1 = currentsubNode1->next_sub;
                    // printf("yes\n");
                }
                // no_proper_size_found = false;
                currentNode = currentNode->next;
            }
        }

        // if (currentNode == NULL)
        // {
        //     no_proper_size_found = true;
        // }
    }
    if (no_proper_size_found)
    {
        // printf();
        // printf("saari main node chotti hai\n");
        count++;

        new_main_node = (struct freeList_Main_Node *)mmap(NULL, sizeof(struct freeList_Main_Node), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (new_main_node == MAP_FAILED)
        {
            perror("mmap");
            exit(EXIT_FAILURE);
        }
        pagesused += total_pages_required;
        void *addressof_allocatedMemory_addedafterhead = mmap(NULL, new_mainNode_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (addressof_allocatedMemory_addedafterhead == MAP_FAILED)
        {
            perror("mmap");
            exit(EXIT_FAILURE);
        }
        new_main_node->size = new_mainNode_size;
        new_main_node->physicalAddress_Start = addressof_allocatedMemory_addedafterhead;
        new_main_node->physicalAddress_End = new_main_node->physicalAddress_Start + new_mainNode_size - 1;
        new_main_node->next = NULL;     // Ensure that the 'next' of the new node points to NULL
        new_main_node->prev = maintail; // Set the previous of the new node to the current 'maintail'

        maintail->next = new_main_node; // Update the 'next' of the existing 'maintail' to the new node
        new_main_node->virtualAddress_Start = maintail->virtualAddress_End + 1;
        new_main_node->virtualAddress_End = new_main_node->virtualAddress_Start + (new_main_node->physicalAddress_End - new_main_node->physicalAddress_Start);
        maintail = new_main_node; // Update the 'maintail' to point to the new node

        newsegment = (struct subList_sub_Node *)mmap(NULL, sizeof(struct subList_sub_Node), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        new_main_node->sub_head = newsegment;
        new_main_node->sub_tail = newsegment;

        newsegment->size = size_asked;
        newsegment->physicalAddress_subStart = addressof_allocatedMemory_addedafterhead;
        newsegment->physicalAddress_subEnd = newsegment->physicalAddress_subStart + newsegment->size - 1;
        newsegment->virtualAddress_subStart = new_main_node->virtualAddress_Start;
        newsegment->virtualAddress_subEnd = new_main_node->virtualAddress_Start + (newsegment->physicalAddress_subEnd - newsegment->physicalAddress_subStart);
        newsegment->type = PROCESS;

        newsegment2 = (struct subList_sub_Node *)mmap(NULL, sizeof(struct subList_sub_Node), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        new_main_node->sub_tail = newsegment2;
        newsegment2->type = HOLE;
        newsegment2->size = new_mainNode_size - size_asked;
        newsegment2->physicalAddress_subStart = newsegment->physicalAddress_subEnd + 1;
        newsegment2->physicalAddress_subEnd = newsegment2->physicalAddress_subStart + newsegment2->size - 1;
        newsegment2->virtualAddress_subStart = newsegment->virtualAddress_subEnd + 1;
        newsegment2->virtualAddress_subEnd = newsegment2->virtualAddress_subStart + (newsegment2->physicalAddress_subEnd - newsegment2->physicalAddress_subStart);
        //! Hole ke paas bhi memory haii;

        // newsegment->prev_sub = currentNode;

        newsegment->next_sub = newsegment2;

        newsegment2->prev_sub = newsegment;
        newsegment2->next_sub = NULL;

        return (void *)(newsegment->virtualAddress_subStart);
    }
}
//! Helper function for debugging
void printFreeList()
{
    struct freeList_Main_Node *currentNode = mainhead;

    while (currentNode != NULL)
    {
        printf("Main Chain Node: Size: %d, Address: %p, Prev: %p, Next: %p\n",
               currentNode->size, currentNode, currentNode->prev, currentNode->next);

        struct subList_sub_Node *subChainNode = currentNode->sub_head;

        while (subChainNode != NULL)
        {
            printf("\t Prev: %p, Next: %p, Type: %s, Size: %d, ",
                   subChainNode->prev_sub, subChainNode->next_sub,
                   (subChainNode->type == PROCESS) ? "PROCESS" : "HOLE", subChainNode->size);

            // Print physical addresses in unsigned int format
            printf("Physical Address: %u, %u\n",
                   (unsigned int)(uintptr_t)subChainNode->physicalAddress_subStart,
                   (unsigned int)(uintptr_t)subChainNode->physicalAddress_subEnd);

            subChainNode = subChainNode->next_sub;
        }

        currentNode = currentNode->next;
    }
}

// Sub Chain Node: Virtual Address: %p, Physical Address: %p,
/*
this function print the stats of the MeMS system like
1. How many pages are utilised by using the mems_malloc
2. how much memory is unused i.e. the memory that is in freelist and is not used.
3. It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.
Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT
*/
void mems_print_stats()
{
    int arr[1000] = {};
    int mainchainlength = 0;
    int subchainlength = 0;
    int memory_unused = 0;
    struct freeList_Main_Node *new_traversal_main_node = head;
    printf("--------MeMS SYSTEM STATS --------\n");

    while (new_traversal_main_node != NULL)
    {
        printf("MAIN[%d:%d]->", new_traversal_main_node->virtualAddress_Start, new_traversal_main_node->virtualAddress_End);

        struct subList_sub_Node *new_traversal_sub_node = new_traversal_main_node->sub_head;

        while (new_traversal_sub_node != NULL)
        {
            subchainlength++;
            if (new_traversal_sub_node->type == PROCESS)
            {
                printf("P[%d:%d]", new_traversal_sub_node->virtualAddress_subStart, new_traversal_sub_node->virtualAddress_subEnd);
            }
            else if (new_traversal_sub_node->type == HOLE)
            {
                memory_unused += new_traversal_sub_node->virtualAddress_subEnd - new_traversal_sub_node->virtualAddress_subStart + 1;
                printf("H[%d:%d]", new_traversal_sub_node->virtualAddress_subStart, new_traversal_sub_node->virtualAddress_subEnd);
            }
            printf("<->");
            new_traversal_sub_node = new_traversal_sub_node->next_sub;
        }
        arr[mainchainlength] = subchainlength;
        printf("NULL\n");
        mainchainlength++;

        new_traversal_main_node = new_traversal_main_node->next;
        subchainlength = 0;
    }
    printf("Pages used:         %d\n", pagesused);
    printf("Space unused:      %d\n", memory_unused);
    printf("Main Chain Length:      %d\n", mainchainlength);
    printf("Sub-chain Length array: [");
    for (int i = 0; i < mainchainlength; i++)
    {
        printf("%d,", arr[i]);
    }
    printf(" ]\n");

    printf("--------------------------------");
    printf("\n");
}

/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/
// void *mems_get(void *v_ptr)
// {

//     void *mems_get(void *v_ptr)
//     {

//         struct freeList_Main_Node *new_traversal_main_node = head;

//         while (new_traversal_main_node != NULL)
//         {
//             struct subList_sub_Node *new_traversal_sub_node = new_traversal_main_node->sub_head;

//             while (new_traversal_sub_node != NULL)
//             {
//                 // Assuming each segment of memory holds the start and end addresses
//                 // Determine if the virtual address is within this segment
//                 if (new_traversal_sub_node->virtualAddress_subStart == v_ptr)
//                 {

//                     return (void *)new_traversal_sub_node->physicalAddress_subStart; // Return the physical address
//                 }

//                 new_traversal_sub_node = new_traversal_sub_node->next_sub;
//             }

//             new_traversal_main_node = new_traversal_main_node->next;
//         }

//         return NULL; // If virtual address is not found, return NULL
//     }
// }
void *mems_get(void *v_ptr)
{
    struct freeList_Main_Node *new_traversal_main_node = head;

    while (new_traversal_main_node != NULL)
    {
        struct subList_sub_Node *new_traversal_sub_node = new_traversal_main_node->sub_head;

        while (new_traversal_sub_node != NULL)
        {
            if (v_ptr >= new_traversal_sub_node->virtualAddress_subStart &&
                v_ptr < (new_traversal_sub_node->virtualAddress_subStart + new_traversal_sub_node->size))
            {
                // Calculate the offset for the physical address
                size_t offset = (size_t)v_ptr - (size_t)new_traversal_sub_node->virtualAddress_subStart;
                // Return the corresponding physical address
                return (void *)((size_t)new_traversal_sub_node->physicalAddress_subStart + offset);
            }
            new_traversal_sub_node = new_traversal_sub_node->next_sub;
        }
        new_traversal_main_node = new_traversal_main_node->next;
    }
    return NULL;
}

/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: nothing
*/
void mems_free(void *v_ptr)
{
    struct freeList_Main_Node *new_traversal_main_node = head;

    while (new_traversal_main_node != NULL)
    {
        struct subList_sub_Node *new_traversal_sub_node = new_traversal_main_node->sub_head;

        while (new_traversal_sub_node != NULL)
        {

            if (new_traversal_sub_node->virtualAddress_subStart == v_ptr)
            {

                //printf("Yipee\n");
                if (new_traversal_sub_node->type == HOLE)
                {
                    //printf("Oof\n");
                    break;
                }
                else if (new_traversal_sub_node->type == PROCESS)
                {
                    //printf("aagya\n");
                    new_traversal_sub_node->type = HOLE;
                    if (new_traversal_sub_node == new_traversal_main_node->sub_head)
                    {
                        //rintf("balle balle\n");
                        if (new_traversal_sub_node->next_sub->type == HOLE)
                        {
                            if (new_traversal_sub_node->next_sub->next_sub == NULL)
                            {
                                //printf("Y4\n");
                                new_traversal_main_node->sub_tail = new_traversal_sub_node;
                                new_traversal_sub_node->physicalAddress_subEnd = new_traversal_sub_node->next_sub->physicalAddress_subEnd;
                                new_traversal_sub_node->virtualAddress_subEnd = new_traversal_sub_node->next_sub->virtualAddress_subEnd;
                                new_traversal_sub_node->size += new_traversal_sub_node->next_sub->size;
                                new_traversal_sub_node->next_sub = NULL;
                            }
                            else
                            {
                                //printf("Y5\n");
                                //printf("Kyu\n");

                                new_traversal_sub_node->physicalAddress_subEnd = new_traversal_sub_node->next_sub->physicalAddress_subEnd;
                                new_traversal_sub_node->virtualAddress_subEnd = new_traversal_sub_node->next_sub->virtualAddress_subEnd;
                                new_traversal_sub_node->size += new_traversal_sub_node->next_sub->size;
                                new_traversal_sub_node->next_sub->next_sub->prev_sub = new_traversal_sub_node->next_sub;
                                new_traversal_sub_node->next_sub = new_traversal_sub_node->next_sub->next_sub;
                            }
                        }
                    }
                    else if (new_traversal_sub_node->next_sub == NULL)
                    {
                        //printf("Y3\n");
                        if (new_traversal_sub_node->next_sub == NULL)
                        {
                            //printf("iii\n");
                            new_traversal_sub_node->type=HOLE;
                            new_traversal_main_node->sub_tail = new_traversal_sub_node->prev_sub;
                            //printf("%d %d\n", new_traversal_sub_node->prev_sub->virtualAddress_subStart, new_traversal_sub_node->prev_sub->virtualAddress_subEnd);
                            //printf("%d %d\n", new_traversal_sub_node->virtualAddress_subStart, new_traversal_sub_node->virtualAddress_subEnd);
                            new_traversal_sub_node->prev_sub->next_sub = NULL;
                            //printf("%d %d\n", new_traversal_sub_node->prev_sub->next_sub->virtualAddress_subStart, new_traversal_sub_node->prev_sub->next_sub->virtualAddress_subEnd);
                            new_traversal_sub_node->prev_sub->physicalAddress_subEnd = new_traversal_sub_node->physicalAddress_subEnd;
                            new_traversal_sub_node->prev_sub->virtualAddress_subEnd = new_traversal_sub_node->virtualAddress_subEnd;
                            //printf("%d %d\n", new_traversal_sub_node->prev_sub->virtualAddress_subStart, new_traversal_sub_node->prev_sub->virtualAddress_subEnd);
                            //new_traversal_sub_node->next_sub->prev_sub = new_traversal_sub_node->prev_sub;
                            new_traversal_sub_node->prev_sub->size += new_traversal_main_node->size;
                        }
                        else
                        {
                            //printf("%d %d\n", new_traversal_sub_node->prev_sub->virtualAddress_subStart, new_traversal_sub_node->prev_sub->virtualAddress_subEnd);
                            //printf("%d %d\n", new_traversal_sub_node->virtualAddress_subStart, new_traversal_sub_node->virtualAddress_subEnd);
                            new_traversal_sub_node->prev_sub->next_sub = new_traversal_sub_node->next_sub;
                            //printf("%d %d\n", new_traversal_sub_node->prev_sub->next_sub->virtualAddress_subStart, new_traversal_sub_node->prev_sub->next_sub->virtualAddress_subEnd);
                            new_traversal_sub_node->prev_sub->physicalAddress_subEnd = new_traversal_sub_node->physicalAddress_subEnd;
                            new_traversal_sub_node->prev_sub->virtualAddress_subEnd = new_traversal_sub_node->virtualAddress_subEnd;
                            //printf("%d %d\n", new_traversal_sub_node->prev_sub->virtualAddress_subStart, new_traversal_sub_node->prev_sub->virtualAddress_subEnd);
                            new_traversal_sub_node->next_sub->prev_sub = new_traversal_sub_node->prev_sub;
                            new_traversal_sub_node->prev_sub->size += new_traversal_main_node->size;
                        }
                    }
                    else if (new_traversal_sub_node->prev_sub->type == HOLE && new_traversal_sub_node->next_sub->type == HOLE)
                    {
                        if (new_traversal_sub_node->next_sub->next_sub == NULL)
                        {
                            //printf("Y1\n");
                            new_traversal_main_node->sub_tail = new_traversal_sub_node->prev_sub;
                            new_traversal_sub_node->prev_sub->next_sub = new_traversal_sub_node->next_sub;
                            new_traversal_sub_node->next_sub->prev_sub = new_traversal_sub_node->prev_sub;
                            new_traversal_sub_node->prev_sub->physicalAddress_subEnd = new_traversal_sub_node->physicalAddress_subEnd;
                            new_traversal_sub_node->prev_sub->virtualAddress_subEnd = new_traversal_sub_node->virtualAddress_subEnd;
                            // new_traversal_sub_node->next_sub->next_sub->prev_sub = new_traversal_sub_node->prev_sub;
                            new_traversal_sub_node->prev_sub->physicalAddress_subEnd = new_traversal_sub_node->next_sub->physicalAddress_subEnd;
                            new_traversal_sub_node->prev_sub->virtualAddress_subEnd = new_traversal_sub_node->next_sub->virtualAddress_subEnd;
                            new_traversal_sub_node->prev_sub->size += new_traversal_sub_node->next_sub->size;
                            new_traversal_sub_node->prev_sub->next_sub = new_traversal_sub_node->next_sub->next_sub;
                            new_traversal_sub_node->prev_sub->size += new_traversal_main_node->size;
                        }
                        else
                        {
                            //printf("Y2\n");
                            new_traversal_sub_node->prev_sub->next_sub = new_traversal_sub_node->next_sub;
                            new_traversal_sub_node->next_sub->prev_sub = new_traversal_sub_node->prev_sub;
                            new_traversal_sub_node->prev_sub->physicalAddress_subEnd = new_traversal_sub_node->physicalAddress_subEnd;
                            new_traversal_sub_node->prev_sub->virtualAddress_subEnd = new_traversal_sub_node->virtualAddress_subEnd;
                            new_traversal_sub_node->prev_sub->size += new_traversal_main_node->size;

                            new_traversal_sub_node->prev_sub->next_sub = new_traversal_sub_node->next_sub->next_sub;
                            new_traversal_sub_node->next_sub->next_sub->prev_sub = new_traversal_sub_node->prev_sub;
                            new_traversal_sub_node->prev_sub->physicalAddress_subEnd = new_traversal_sub_node->next_sub->physicalAddress_subEnd;
                            new_traversal_sub_node->prev_sub->virtualAddress_subEnd = new_traversal_sub_node->next_sub->virtualAddress_subEnd;
                            new_traversal_sub_node->prev_sub->size += new_traversal_sub_node->next_sub->size;
                        }
                    }
                    else if (new_traversal_sub_node->prev_sub->type == HOLE)
                    {
                        //printf("Y3\n");
                        if (new_traversal_sub_node->next_sub == NULL)
                        {
                            new_traversal_main_node->sub_tail = new_traversal_sub_node->prev_sub;
                        }
                        //printf("%d %d\n", new_traversal_sub_node->prev_sub->virtualAddress_subStart, new_traversal_sub_node->prev_sub->virtualAddress_subEnd);
                        //printf("%d %d\n", new_traversal_sub_node->virtualAddress_subStart, new_traversal_sub_node->virtualAddress_subEnd);
                        new_traversal_sub_node->prev_sub->next_sub = new_traversal_sub_node->next_sub;
                        //printf("%d %d\n", new_traversal_sub_node->prev_sub->next_sub->virtualAddress_subStart, new_traversal_sub_node->prev_sub->next_sub->virtualAddress_subEnd);
                        new_traversal_sub_node->prev_sub->physicalAddress_subEnd = new_traversal_sub_node->physicalAddress_subEnd;
                        new_traversal_sub_node->prev_sub->virtualAddress_subEnd = new_traversal_sub_node->virtualAddress_subEnd;
                        //printf("%d %d\n", new_traversal_sub_node->prev_sub->virtualAddress_subStart, new_traversal_sub_node->prev_sub->virtualAddress_subEnd);
                        new_traversal_sub_node->next_sub->prev_sub = new_traversal_sub_node->prev_sub;
                        new_traversal_sub_node->prev_sub->size += new_traversal_main_node->size;
                    }
                    else if (new_traversal_sub_node->next_sub->type == HOLE)
                    {
                        if (new_traversal_sub_node->next_sub->next_sub == NULL)
                        {
                            //printf("Y4\n");
                            new_traversal_main_node->sub_tail = new_traversal_sub_node;
                            new_traversal_sub_node->physicalAddress_subEnd = new_traversal_sub_node->next_sub->physicalAddress_subEnd;
                            new_traversal_sub_node->virtualAddress_subEnd = new_traversal_sub_node->next_sub->virtualAddress_subEnd;
                            new_traversal_sub_node->size += new_traversal_sub_node->next_sub->size;
                            new_traversal_sub_node->next_sub = NULL;
                        }
                        else
                        {
                            //printf("Y5\n");
                            new_traversal_sub_node->next_sub = new_traversal_sub_node->next_sub->next_sub;

                            new_traversal_sub_node->next_sub->next_sub->prev_sub = new_traversal_sub_node->next_sub;

                            new_traversal_sub_node->physicalAddress_subEnd = new_traversal_sub_node->next_sub->physicalAddress_subEnd;
                            new_traversal_sub_node->virtualAddress_subEnd = new_traversal_sub_node->next_sub->virtualAddress_subEnd;
                            new_traversal_sub_node->size += new_traversal_sub_node->next_sub->size;
                        }
                    }
                }
            }

            new_traversal_sub_node = new_traversal_sub_node->next_sub;
        }

        new_traversal_main_node = new_traversal_main_node->next;
    }
}
