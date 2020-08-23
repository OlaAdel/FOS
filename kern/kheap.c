#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>


int NumberOfHeapPages = (KERNEL_HEAP_MAX - KERNEL_HEAP_START)/PAGE_SIZE;
int HeapChunks[(KERNEL_HEAP_MAX - KERNEL_HEAP_START)/PAGE_SIZE];

//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kmalloc()
	// Write your code here, remove the panic and write your code
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	//NOTE: Allocation is based on BEST FIT strategy
	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details


	//TODO: [PROJECT 2019 - BONUS1] Implement the FIRST FIT strategy for Kernel allocation
	// Beside the BEST FIT
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	int MinimumSize = 1e9;
	int VirutalAddressIndex = -1;
	int ContinuousPageCounter = 0;
	int NumberOfAllocatedPages = size/PAGE_SIZE;
	if(size % PAGE_SIZE != 0)++NumberOfAllocatedPages;
	for(int i=0;i<NumberOfHeapPages;++i)
	{
		if(HeapChunks[i] != 0)
		{
			if(ContinuousPageCounter != 0)
			{
				if(ContinuousPageCounter >= NumberOfAllocatedPages && ContinuousPageCounter < MinimumSize)
				{
					MinimumSize = ContinuousPageCounter;
					VirutalAddressIndex = i - ContinuousPageCounter;
				}
				ContinuousPageCounter = 0;
			}
			i += HeapChunks[i]-1;
		}
		else
		{
			++ContinuousPageCounter;
		}
	}
	if(ContinuousPageCounter != 0)
	{
		if(ContinuousPageCounter >= NumberOfAllocatedPages && ContinuousPageCounter < MinimumSize)
		{
			MinimumSize = ContinuousPageCounter;
			VirutalAddressIndex = NumberOfHeapPages - ContinuousPageCounter;

		}
	}
	if(VirutalAddressIndex != -1)
	{
		int virtualAddress = KERNEL_HEAP_START + (VirutalAddressIndex*PAGE_SIZE);
		HeapChunks[VirutalAddressIndex] = NumberOfAllocatedPages;
		int ReturnAddress = virtualAddress;
		int CurrentAddress = virtualAddress;
		for(int i=0;i<NumberOfAllocatedPages;++i)
		{
			struct Frame_Info* FrameInfoPointer = NULL;
			allocate_frame(&FrameInfoPointer);
			map_frame(ptr_page_directory, FrameInfoPointer, (void*)CurrentAddress, PERM_PRESENT|PERM_WRITEABLE);

			CurrentAddress += PAGE_SIZE;
		}
		return (void*)ReturnAddress;
	}

	//change this "return" according to your answer
	return NULL;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kfree()
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details
	int VirutalAddressIndex=(uint32)(virtual_address-KERNEL_HEAP_START)/PAGE_SIZE;
	int CurrentAddress = (uint32)(virtual_address);
	for(int i=0;i<HeapChunks[VirutalAddressIndex];++i)
	{
		uint32* PageTablePointer = NULL;
		struct Frame_Info* FrameInfoPointer = get_frame_info(ptr_page_directory, (void*)CurrentAddress, &PageTablePointer);
		free_frame(FrameInfoPointer);
		unmap_frame(ptr_page_directory, (void*)CurrentAddress);
		CurrentAddress +=PAGE_SIZE;
	}
	HeapChunks[VirutalAddressIndex] = 0;

}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kheap_virtual_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer
	for(int i=0;i<NumberOfHeapPages;++i)
	{
		int VirtualAddress = KERNEL_HEAP_START+(i*PAGE_SIZE);
		uint32* PageTablePointer = NULL;
		get_page_table(ptr_page_directory, (void*)VirtualAddress, &PageTablePointer);
		int FrameNumber = PageTablePointer[PTX(VirtualAddress)];
		FrameNumber>>=12;
		if(FrameNumber*PAGE_SIZE == physical_address)
			return VirtualAddress;
	}

	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kheap_physical_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer
	uint32* PageTablePointer = NULL;
	get_page_table(ptr_page_directory, (uint32*)virtual_address, &PageTablePointer);
	int FrameNumber = PageTablePointer[PTX(virtual_address)];
	FrameNumber >>=12;
	return FrameNumber*PAGE_SIZE;

	return 0;
}


//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2019 - BONUS2] Kernel Heap Realloc
	// Write your code here, remove the panic and write your code

	return NULL;
	panic("krealloc() is not implemented yet...!!");

}
