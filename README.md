                                                     MeMS: Memory Management System [CSE231 OS Assignment 3]


Documentation


---------------------------------------------------------------------------------------------------------------------------------------------------

How to run the exmaple.c
After implementing functions in mems.h follow the below steps to run example.c file


$ make
$ ./example


---------------------------------------------------------------------------------------------------------------------------------------------------


Sub-chain Length array: <Array denoting the length of the subchains>

Example Output
PAGE_SIZE= 4096 Bytes

Starting MeMS Virtual Address= 1000


---------------------------------------------------------------------------------------------------------------------------------------------------

Format of mems_mems_print_stats
For every Subchain in the free list the data is being printed as follows

MAIN[starting_mems_vitual_address:ending_mems_vitual_address] -> <HOLE or PROCESS>[starting_mems_vitual_address:ending_mems_vitual_address] <-> ..... <-> NULL


------------------------------------------------------------------  CODE LOGIC --------------------------------------------------------------------


                                                           Basic Structure and Implementation


Data Structures:

-struct freeList_Main_Node: Represents a main node in the free list.
-prev and next: Pointers to the previous and next main nodes.
-size: Size of the memory block allocated in the node.
-virtualAddress_Start and physicalAddress_Start: Start virtual and physical addresses of the allocated memory.
-virtualAddress_End and physicalAddress_End: End virtual and physical addresses of the allocated memory.
-sub_head and sub_tail: Pointers to the start and end of the sub-list.
-struct subList_sub_Node: Represents a sub-node in the sub-list.
-prev_sub and next_sub: Pointers to the previous and next sub-nodes.
-virtualAddress_subStart and physicalAddress_subStart: Start virtual and physical addresses of the sub-node.
-virtualAddress_subEnd and physicalAddress_subEnd: End virtual and physical addresses of the sub-node.
-type: Indicates whether the sub-node is a HOLE or PROCESS type.
-size: Size of the memory block allocated in the sub-node.
-Functionality:
-mems_init(): Initializes the MeMS system.

Allocates the initial main node and sets up the structure for the MeMS system.
mems_finish(): Cleans up the MeMS system.

Unmaps allocated memory used by the MeMS system.
mems_malloc(size_t size_asked): Allocates memory in the MeMS system.

Searches for available memory in the free list and, if needed, allocates more memory using mmap.
Manages the main and sub-nodes to represent allocated and free memory segments.
mems_get(void *v_ptr): Returns the physical address mapped to a given virtual address.

mems_free(void *v_ptr): Frees up memory pointed by a virtual address and adds it back to the free list.

mems_print_stats(): Displays statistics about memory usage in the MeMS system.


Enumerated Types:

enum NodeType: Defines the types of nodes within the MeMS system - HOLE (unused memory) and PROCESS (allocated memory).


---------------------------------------------------------------------------------------------------------------------------------------------------
                                                      Function description for each function 
---------------------------------------------------------------------------------------------------------------------------------------------------

(a)void mems_init()

Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing


********************************************************  How it is implemented - *****************************************************************


Memory Allocation:

Utilizes mmap() to allocate memory for the initial head node, which represents the start of the MeMS system.
Initializes the free list, which is a linked structure comprising nodes for managing memory segments.
Main Node Initialization:

The head node is created, containing information about the initial memory allocation for the MeMS system.
Sets the prev, next, sub_head, and sub_tail pointers to NULL to signify an empty structure.
The maintail and mainhead pointers are set to point to the created head node.
Error Handling:

Error checking for the success of mmap() allocation is implemented. If the allocation fails, the program exits with an error message.
Logging (Debug Mode):

Provides debug logs about the size, address, and pointers of the initialized free list nodes for debugging purposes.


---------------------------------------------------------------------------------------------------------------------------------------------------


(b)void mems_finish()

This function will be called at the end of the MeMS system and its main job is to unmap the 
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/

*********************************************************  How it is implemented - *****************************************************************

The purpose of the mems_finish() function is to free all the memory allocated during the MeMS execution. The function works as follows:

Traversal of Main Nodes:

It starts by traversing through the main nodes of the MeMS structure.
Deallocating Sub-Nodes:

For each main node, it iterates through the sub-nodes attached to that main node.
It frees the memory allocated for each sub-node using munmap. This deallocates the specific memory segments represented by the sub-nodes.
Deallocating Main Nodes:

After freeing all associated sub-nodes, the function deallocates the memory associated with each main node.
It uses munmap to release the memory allocated for the main nodes themselves.
Complete Cleanup:

This process ensures the entire memory used by the MeMS system is deallocated and returned to the system.


PSEUDOCODE EXPLANATION (logic building)

mems_finish():
    traversal_main_node = head

    while traversal_main_node is not NULL:
        temp_main_node = traversal_main_node
        traversal_sub_node = traversal_main_node->sub_head

        while traversal_sub_node is not NULL:
            temp_sub_node = traversal_sub_node

            munmap(temp_sub_node->physicalAddress_subStart, temp_sub_node->size)
            munmap(temp_sub_node, sizeof(struct subList_sub_Node))

            traversal_sub_node = traversal_sub_node->next_sub

        munmap(temp_main_node->physicalAddress_Start, temp_main_node->size)
        munmap(temp_main_node, sizeof(struct freeList_Main_Node))

        traversal_main_node = traversal_main_node->next



---------------------------------------------------------------------------------------------------------------------------------------------------


(c)void* mems_malloc(size_t size)

Allocates memory of the specified size by reusing a segment from the free list if 
a sufficiently large segment is available. 

Else, uses the mmap system call to allocate more memory on the heap and updates 
the free list accordingly.

Note that while mapping using mmap do not forget to reuse the unused space from mapping
by adding it to the free list.
Parameter: The size of the memory the user program wants
Returns: MeMS Virtual address (that is created by MeMS)


********************************************************  How it is implemented - *****************************************************************


The code uses a linked list structure for memory allocation. There are two main types of nodes: freeList_Main_Node and subList_sub_Node.

freeList_Main_Node: Represents a main memory block and contains information about allocated and free segments within the block.
subList_sub_Node: Represents a subsegment within the main memory block. It tracks allocated and free memory segments.


The mems_malloc function is responsible for allocating memory and maintaining the free list. It takes the size of memory requested as a parameter and returns the virtual address for the allocated memory. The function works as follows:

Initialization: It initializes necessary variables and checks the availability of free memory.

Finding Suitable Free Block:

It iterates through the linked list of main nodes to find a suitable free block for the requested memory size.
If an appropriate block is not found, it allocates a new main node using mmap to obtain a new memory block.
Allocating Memory:

If an appropriate free block is found, the function allocates memory within that block.
It updates the free list by dividing the block into used and unused segments (if needed).
Return:

The function returns the virtual address of the allocated memory.

The function iterates through the free list to find the appropriate block with enough memory to fulfill the request.
It uses the mmap system call to allocate memory and updates the linked list nodes to represent the allocated memory segments.





---------------------------------------------------------------------------------------------------------------------------------------------------


(d)void mems_print_stats()



this function print the stats of the MeMS system like
1. How many pages are utilised by using the mems_malloc
2. how much memory is unused i.e. the memory that is in freelist and is not used.
3. It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.
Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT



********************************************************  How it is implemented - *****************************************************************


Purpose:
Display MeMS system statistics and details about main and sub-chains.
Gather information on utilized pages, unused memory, main chain length, and sub-chain lengths.
Steps and Functionalities:
Initialization:

Initialize variables to keep track of various statistics, such as arr, mainchainlength, subchainlength, and memory_unused.
Traversal through Main Chain:

Iterates through the main chain of the MeMS system (new_traversal_main_node = head).
Prints details of each main chain node, including the start and end virtual addresses.
While traversing, enters the sub-chain.
Traversal through Sub-Chains:

Traverses the sub-chain of each main node (new_traversal_sub_node = new_traversal_main_node->sub_head).
Prints details of each sub-chain node:
Prints the type of node (PROCESS or HOLE).
Updates statistics such as subchainlength and memory_unused based on the node type and its virtual addresses.
Collecting Statistics:

Gathers statistics like pagesused and memory_unused.
Tracks the length of the main chain (mainchainlength).
Records the sub-chain lengths in the arr array.
Printing Stats:

Outputs statistics and details obtained from the MeMS traversal.
Prints pages used, space unused, main chain length, and the sub-chain length array.


The function starts traversing from the main node, examining each sub-node to locate the specific segment that includes the provided virtual address.
Once the segment is identified, it computes the offset within that segment and translates it to the corresponding physical address.
If the virtual address doesn't match any segment, the function returns NULL to indicate the absence of a mapped physical address.




---------------------------------------------------------------------------------------------------------------------------------------------------


(e)void *mems_get(void*v_ptr)



Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).


********************************************************  How it is implemented - *****************************************************************


The mems_get() function is a critical part of the Memory Management System (MeMS) that facilitates the retrieval of the MeMS physical address associated with a provided MeMS virtual address. This function is crucial for translating virtual addresses into their corresponding physical addresses, enabling users to work with memory allocations accurately.

Here's an in-depth explanation of the mems_get() function:

Objective:
The primary goal of mems_get() is to find and return the MeMS physical address associated with a given MeMS virtual address.

Functionality:
Input Parameters:

The function takes a void *v_ptr parameter, representing the MeMS virtual address whose corresponding physical address needs to be determined.
Traversal through Main and Sub-Node Chains:

It initiates a traversal through the MeMS's main nodes to search for the segment that contains the provided virtual address.
Segment Matching:

For each main node, it traverses the sub-nodes associated with that main node.
Within each sub-node, the function compares the provided virtual address against the range of addresses covered by that sub-node to determine if the address lies within that segment.
Address Translation:

If the provided virtual address falls within the range of a particular sub-node, the function calculates the offset within that sub-node.
It then adds this offset to the starting physical address of the sub-node, effectively determining the physical address corresponding to the virtual address.
Handling Unavailable Addresses:

If the provided virtual address doesn't match any segments in the MeMS system, the function returns NULL to signify that the given virtual address isn't currently mapped to any physical address.


---------------------------------------------------------------------------------------------------------------------------------------------------


(f)void mems_free(void *v_ptr)



this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS) 
Returns: nothing


********************************************************  How it is implemented - *****************************************************************


Function Purpose:
Objective: To free up memory pointed by a provided virtual address (v_ptr) and manage the free list accordingly.
Data Structures: Utilizes linked list structures for main nodes and sub-nodes to represent memory segments.
Explanation of Implementation:
Traversal through Main Nodes:

The function initiates traversal through the main nodes of the free list.
For each main node, it inspects the sub-nodes associated with it.
Sub-Node Traversal:

The function iterates through the sub-nodes within each main node to find the target memory segment pointed by the given virtual address (v_ptr).
Freeing Memory:

Upon locating the target sub-node, the function checks its type (HOLE or PROCESS).
If the sub-node type is PROCESS (indicating allocated memory), it changes the type to HOLE, marking the memory as available for reuse.
Consolidating Holes (Free Memory):

After marking the memory as free (HOLE), the function checks neighboring sub-nodes to determine if they are also HOLE type.
If contiguous free memory segments are found, it consolidates them by merging into a single larger HOLE segment.
Adjusting the Free List:

Depending on the scenario encountered (e.g., two neighboring HOLE segments), the function adjusts the pointers and sizes of the nodes to ensure proper management of the free list.(Optimisation Considerations).
This includes redefining sub-node pointers and updating their sizes and physical/virtual address details.
Reorganization and Optimization:

The function aims to optimize the free list, ensuring efficient utilization of memory space by consolidating adjacent free segments.
It also adjusts pointers and sizes to reflect changes in the memory layout.


PSUEDOCODE ( Logic Building )


mems_free(void *v_ptr):
    for each main_node in free list:
        for each sub_node in main_node:
            if sub_node's virtual address matches v_ptr:
                if sub_node is of type PROCESS:
                    change sub_node's type to HOLE
                    # Check adjacent nodes for hole merging
                    if neighboring nodes are also HOLE:
                        consolidate neighboring HOLE segments
                    # Adjust the free list and pointers
                    adjust main_node and sub_node pointers


---------------------------------------------------------------------------------------------------------------------------------------------------
	

---- Included files ----


#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>


---------------------------------------------------------------------------------------------------------------------------------------------------



********************************************************  Additional Considerations- **************************************************************

In mems_finish() - we are deallocating space occupied by the structure too -- complete clean up - mentioned in the functionality too

There is a display of two warnings, it has nothing to do with the code implementation

Edge cases considered -  (like joining 2 adjacent hole nodes to make a new hole node)

Optimisation consideration in mems_free()


---------------------------------------------------------------------------------------------------------------------------------------------------

                                                                                Self Generated Test cases 

Example case 1 -

(Example.c)

---------------------------------------------------------------------------------------------------------------------------------------------------

Example Code 1 -

// include other header files as needed
#include"mems.h"


int main(int argc, char const *argv[])
{
    // initialise the MeMS system 
    mems_init();
    int* ptr[10];

    /*
    This allocates 10 arrays of 250 integers each
    */
    printf("\n------- Allocated virtual addresses [mems_malloc] -------\n");
    for(int i=0;i<10;i++){
        ptr[i] = (int*)mems_malloc(sizeof(int)*250);
        printf("Virtual address: %lu\n", (unsigned long)ptr[i]);
    }

    /*
    In this section we are tring to write value to 1st index of array[0] (here it is 0 based indexing).
    We get get value of both the 0th index and 1st index of array[0] by using function mems_get.
    Then we write value to 1st index using 1st index pointer and try to access it via 0th index pointer.

    This section is show that even if we have allocated an array using mems_malloc but we can 
    retrive MeMS physical address of any of the element from that array using mems_get. 
    */
    printf("\n------ Assigning value to Virtual address [mems_get] -----\n");
    // how to write to the virtual address of the MeMS (this is given to show that the system works on arrays as well)
    int* phy_ptr= (int*) mems_get(&ptr[0][1]); // get the address of index 1
    phy_ptr[0]=200; // put value at index 1
    int* phy_ptr2= (int*) mems_get(&ptr[0][0]); // get the address of index 0
    printf("Virtual address: %lu\tPhysical Address: %lu\n",(unsigned long)ptr[0],(unsigned long)phy_ptr2);
    printf("Value written: %d\n", phy_ptr2[1]); // print the address of index 1 

    /*
    This shows the stats of the MeMS system.  
    */
    printf("\n--------- Printing Stats [mems_print_stats] --------\n");
    mems_print_stats();

    /*
    This section shows the effect of freeing up space on free list and also the effect of 
    reallocating the space that will be fullfilled by the free list.
    */
    printf("\n--------- Freeing up the memory [mems_free] --------\n");
    mems_free(ptr[2]);
    mems_print_stats();
    mems_free(ptr[3]);
    mems_print_stats();
    
    ptr[3] = (int*)mems_malloc(sizeof(int)*250);
    mems_print_stats();

    printf("\n--------- Unmapping all memory [mems_finish] --------\n\n");
    mems_finish();
    return 0;
}

---------------------------------------------------------------------------------------------------------------------------------------------------

Output 

Example Output 1 -

![image](https://github.com/palak-b19/Memory-Manager/assets/119069053/b578d142-6ac2-4a64-99fc-3eafeb161d61)



example.c


--- Resources referred ---

**************************************

Doubt session 2 - By professor Dhruv Kumar(IIITD)




***************************************
