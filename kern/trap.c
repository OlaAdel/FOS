#include <inc/mmu.h>
#include <inc/x86.h>
#include <inc/assert.h>

#include <kern/memory_manager.h>
#include <kern/trap.h>
#include <kern/console.h>
#include <kern/command_prompt.h>
#include <kern/user_environment.h>
#include <kern/file_manager.h>
#include <kern/syscall.h>
#include <kern/sched.h>
#include <kern/kclock.h>
#include <kern/trap.h>

extern void __static_cpt(uint32 *ptr_page_directory, const uint32 virtual_address, uint32 **ptr_page_table);

void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va);
void page_fault_handler(struct Env * curenv, uint32 fault_va);
void table_fault_handler(struct Env * curenv, uint32 fault_va);

static struct Taskstate ts;

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;


/// Interrupt descriptor table.  (Must be built at run time because
/// shifted function addresses can't be represented in relocation records.)
///

struct Gatedesc idt[256] = { { 0 } };
struct Pseudodesc idt_pd = {
		sizeof(idt) - 1, (uint32) idt
};
extern  void (*PAGE_FAULT)();
extern  void (*SYSCALL_HANDLER)();
extern  void (*DBL_FAULT)();

extern  void (*ALL_FAULTS0)();
extern  void (*ALL_FAULTS1)();
extern  void (*ALL_FAULTS2)();
extern  void (*ALL_FAULTS3)();
extern  void (*ALL_FAULTS4)();
extern  void (*ALL_FAULTS5)();
extern  void (*ALL_FAULTS6)();
extern  void (*ALL_FAULTS7)();
//extern  void (*ALL_FAULTS8)();
//extern  void (*ALL_FAULTS9)();
extern  void (*ALL_FAULTS10)();
extern  void (*ALL_FAULTS11)();
extern  void (*ALL_FAULTS12)();
extern  void (*ALL_FAULTS13)();
//extern  void (*ALL_FAULTS14)();
//extern  void (*ALL_FAULTS15)();
extern  void (*ALL_FAULTS16)();
extern  void (*ALL_FAULTS17)();
extern  void (*ALL_FAULTS18)();
extern  void (*ALL_FAULTS19)();


extern  void (*ALL_FAULTS32)();
extern  void (*ALL_FAULTS33)();
extern  void (*ALL_FAULTS34)();
extern  void (*ALL_FAULTS35)();
extern  void (*ALL_FAULTS36)();
extern  void (*ALL_FAULTS37)();
extern  void (*ALL_FAULTS38)();
extern  void (*ALL_FAULTS39)();
extern  void (*ALL_FAULTS40)();
extern  void (*ALL_FAULTS41)();
extern  void (*ALL_FAULTS42)();
extern  void (*ALL_FAULTS43)();
extern  void (*ALL_FAULTS44)();
extern  void (*ALL_FAULTS45)();
extern  void (*ALL_FAULTS46)();
extern  void (*ALL_FAULTS47)();



static const char *trapname(int trapno)
{
	static const char * const excnames[] = {
			"Divide error",
			"Debug",
			"Non-Maskable Interrupt",
			"Breakpoint",
			"Overflow",
			"BOUND Range Exceeded",
			"Invalid Opcode",
			"Device Not Available",
			"Double Fault",
			"Coprocessor Segment Overrun",
			"Invalid TSS",
			"Segment Not Present",
			"Stack Fault",
			"General Protection",
			"Page Fault",
			"(unknown trap)",
			"x87 FPU Floating-Point Error",
			"Alignment Check",
			"Machine-Check",
			"SIMD Floating-Point Exception"
	};

	if (trapno < sizeof(excnames)/sizeof(excnames[0]))
		return excnames[trapno];
	if (trapno == T_SYSCALL)
		return "System call";
	return "(unknown trap)";
}


void
idt_init(void)
{
	extern struct Segdesc gdt[];

	// LAB 3: Your code here.
	//initialize idt
	SETGATE(idt[T_PGFLT], 0, GD_KT , &PAGE_FAULT, 0) ;
	SETGATE(idt[T_SYSCALL], 0, GD_KT , &SYSCALL_HANDLER, 3) ;
	SETGATE(idt[T_DBLFLT], 0, GD_KT , &DBL_FAULT, 0) ;


	SETGATE(idt[T_DIVIDE   ], 0, GD_KT , &ALL_FAULTS0, 3) ;
	SETGATE(idt[T_DEBUG    ], 1, GD_KT , &ALL_FAULTS1, 3) ;
	SETGATE(idt[T_NMI      ], 0, GD_KT , &ALL_FAULTS2, 3) ;
	SETGATE(idt[T_BRKPT    ], 1, GD_KT , &ALL_FAULTS3, 3) ;
	SETGATE(idt[T_OFLOW    ], 1, GD_KT , &ALL_FAULTS4, 3) ;
	SETGATE(idt[T_BOUND    ], 0, GD_KT , &ALL_FAULTS5, 3) ;
	SETGATE(idt[T_ILLOP    ], 0, GD_KT , &ALL_FAULTS6, 3) ;
	SETGATE(idt[T_DEVICE   ], 0, GD_KT , &ALL_FAULTS7, 3) ;
	//SETGATE(idt[T_DBLFLT   ], 0, GD_KT , &ALL_FAULTS, 3) ;
	//SETGATE(idt[], 0, GD_KT , &ALL_FAULTS, 3) ;
	SETGATE(idt[T_TSS      ], 0, GD_KT , &ALL_FAULTS10, 3) ;
	SETGATE(idt[T_SEGNP    ], 0, GD_KT , &ALL_FAULTS11, 3) ;
	SETGATE(idt[T_STACK    ], 0, GD_KT , &ALL_FAULTS12, 3) ;
	SETGATE(idt[T_GPFLT    ], 0, GD_KT , &ALL_FAULTS13, 3) ;
	//SETGATE(idt[T_PGFLT    ], 0, GD_KT , &ALL_FAULTS, 3) ;
	//SETGATE(idt[ne T_RES   ], 0, GD_KT , &ALL_FAULTS, 3) ;
	SETGATE(idt[T_FPERR    ], 0, GD_KT , &ALL_FAULTS16, 3) ;
	SETGATE(idt[T_ALIGN    ], 0, GD_KT , &ALL_FAULTS17, 3) ;
	SETGATE(idt[T_MCHK     ], 0, GD_KT , &ALL_FAULTS18, 3) ;
	SETGATE(idt[T_SIMDERR  ], 0, GD_KT , &ALL_FAULTS19, 3) ;


	SETGATE(idt[IRQ0_Clock], 0, GD_KT , &ALL_FAULTS32, 3) ;
	SETGATE(idt[33], 0, GD_KT , &ALL_FAULTS33, 3) ;
	SETGATE(idt[34], 0, GD_KT , &ALL_FAULTS34, 3) ;
	SETGATE(idt[35], 0, GD_KT , &ALL_FAULTS35, 3) ;
	SETGATE(idt[36], 0, GD_KT , &ALL_FAULTS36, 3) ;
	SETGATE(idt[37], 0, GD_KT , &ALL_FAULTS37, 3) ;
	SETGATE(idt[38], 0, GD_KT , &ALL_FAULTS38, 3) ;
	SETGATE(idt[39], 0, GD_KT , &ALL_FAULTS39, 3) ;
	SETGATE(idt[40], 0, GD_KT , &ALL_FAULTS40, 3) ;
	SETGATE(idt[41], 0, GD_KT , &ALL_FAULTS41, 3) ;
	SETGATE(idt[42], 0, GD_KT , &ALL_FAULTS42, 3) ;
	SETGATE(idt[43], 0, GD_KT , &ALL_FAULTS43, 3) ;
	SETGATE(idt[44], 0, GD_KT , &ALL_FAULTS44, 3) ;
	SETGATE(idt[45], 0, GD_KT , &ALL_FAULTS45, 3) ;
	SETGATE(idt[46], 0, GD_KT , &ALL_FAULTS46, 3) ;
	SETGATE(idt[47], 0, GD_KT , &ALL_FAULTS47, 3) ;



	// Setup a TSS so that we get the right stack
	// when we trap to the kernel.
	ts.ts_esp0 = KERNEL_STACK_TOP;
	ts.ts_ss0 = GD_KD;

	// Initialize the TSS field of the gdt.
	gdt[GD_TSS >> 3] = SEG16(STS_T32A, (uint32) (&ts),
			sizeof(struct Taskstate), 0);
	gdt[GD_TSS >> 3].sd_s = 0;

	// Load the TSS
	ltr(GD_TSS);

	// Load the IDT
	asm volatile("lidt idt_pd");
}

void print_trapframe(struct Trapframe *tf)
{
	cprintf("TRAP frame at %p\n", tf);
	print_regs(&tf->tf_regs);
	cprintf("  es   0x----%04x\n", tf->tf_es);
	cprintf("  ds   0x----%04x\n", tf->tf_ds);
	cprintf("  trap 0x%08x %s - %d\n", tf->tf_trapno, trapname(tf->tf_trapno), tf->tf_trapno);
	cprintf("  err  0x%08x\n", tf->tf_err);
	cprintf("  eip  0x%08x\n", tf->tf_eip);
	cprintf("  cs   0x----%04x\n", tf->tf_cs);
	cprintf("  flag 0x%08x\n", tf->tf_eflags);
	cprintf("  esp  0x%08x\n", tf->tf_esp);
	cprintf("  ss   0x----%04x\n", tf->tf_ss);
}

void print_regs(struct PushRegs *regs)
{
	cprintf("  edi  0x%08x\n", regs->reg_edi);
	cprintf("  esi  0x%08x\n", regs->reg_esi);
	cprintf("  ebp  0x%08x\n", regs->reg_ebp);
	cprintf("  oesp 0x%08x\n", regs->reg_oesp);
	cprintf("  ebx  0x%08x\n", regs->reg_ebx);
	cprintf("  edx  0x%08x\n", regs->reg_edx);
	cprintf("  ecx  0x%08x\n", regs->reg_ecx);
	cprintf("  eax  0x%08x\n", regs->reg_eax);
}

static void trap_dispatch(struct Trapframe *tf)
{
	// Handle processor exceptions.
	// LAB 3: Your code here.

	if(tf->tf_trapno == T_PGFLT)
	{
		//print_trapframe(tf);
		if(isPageReplacmentAlgorithmLRU())
		{
			//cprintf("===========Table WS before updating time stamp========\n");
			//env_table_ws_print(curenv) ;
			update_WS_time_stamps();
		}
		fault_handler(tf);
	}
	else if (tf->tf_trapno == T_SYSCALL)
	{
		uint32 ret = syscall(tf->tf_regs.reg_eax
				,tf->tf_regs.reg_edx
				,tf->tf_regs.reg_ecx
				,tf->tf_regs.reg_ebx
				,tf->tf_regs.reg_edi
				,tf->tf_regs.reg_esi);
		tf->tf_regs.reg_eax = ret;
	}
	else if(tf->tf_trapno == T_DBLFLT)
	{
		panic("double fault!!");
	}
	else if (tf->tf_trapno == IRQ0_Clock)
	{
		clock_interrupt_handler() ;
	}

	else
	{
		// Unexpected trap: The user process or the kernel has a bug.
		//print_trapframe(tf);
		if (tf->tf_cs == GD_KT)
		{
			panic("unhandled trap in kernel");
		}
		else {
			//env_destroy(curenv);
			return;
		}
	}
	return;
}
void trap(struct Trapframe *tf)
{
kclock_stop();
int userTrap = 0;
if ((tf->tf_cs & 3) == 3) {
assert(curenv);
curenv->env_tf = *tf;
tf = &(curenv->env_tf);
userTrap = 1;
}
if(tf->tf_trapno == IRQ0_Clock)
{
if (userTrap)
{
assert(curenv);
curenv->nClocks++ ;
}
}
else if (tf->tf_trapno == T_PGFLT){
//2016: Bypass the faulted instruction
if (bypassInstrLength != 0){
if (userTrap){
curenv->env_tf.tf_eip = (uint32*)((uint32)(curenv->env_tf.tf_eip)
+ bypassInstrLength);
env_run(curenv);
}
else{
tf->tf_eip = (uint32*)((uint32)(tf->tf_eip) + bypassInstrLength);
kclock_resume();
env_pop_tf(tf);
}
}
}
trap_dispatch(tf);
if (userTrap)
{
assert(curenv && curenv->env_status == ENV_RUNNABLE);
env_run(curenv);
}
/* 2019
* If trap from kernel, then return to the called kernel function using the passed
param "tf" not the user one that's stored in curenv
*/
else
{
env_pop_tf((tf));
}
}

void setPageReplacmentAlgorithmLRU(){_PageRepAlgoType = PG_REP_LRU;}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}

uint32 isPageReplacmentAlgorithmLRU(){if(_PageRepAlgoType == PG_REP_LRU) return 1; return 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}

void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint32 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint32 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}


void detect_modified_loop()
{
	struct  Frame_Info * slowPtr = LIST_FIRST(&modified_frame_list);
	struct  Frame_Info * fastPtr = LIST_FIRST(&modified_frame_list);


	while (slowPtr && fastPtr) {
		fastPtr = LIST_NEXT(fastPtr); // advance the fast pointer
		if (fastPtr == slowPtr) // and check if its equal to the slow pointer
		{
			cprintf("loop detected in modiflist\n");
			break;
		}

		if (fastPtr == NULL) {
			break; // since fastPtr is NULL we reached the tail
		}

		fastPtr = LIST_NEXT(fastPtr); //advance and check again
		if (fastPtr == slowPtr) {
			cprintf("loop detected in modiflist\n");
			break;
		}

		slowPtr = LIST_NEXT(slowPtr); // advance the slow pointer only once
	}
	cprintf("finished modi loop detection\n");
}

void fault_handler(struct Trapframe *tf)
{
	int userTrap = 0;
	if ((tf->tf_cs & 3) == 3) {
		userTrap = 1;
	}
	//print_trapframe(tf);
	uint32 fault_va;

	// Read processor's CR2 register to find the faulting address
	fault_va = rcr2();

	//2017: Check stack overflow for Kernel
	if (!userTrap)
	{
		if (fault_va < KERNEL_STACK_TOP - KERNEL_STACK_SIZE && fault_va >= USER_LIMIT)
			panic("Kernel: stack overflow exception!");
	}
	//2017: Check stack underflow for User
	else
	{
		if (fault_va >= USTACKTOP)
			panic("User: stack underflow exception!");
	}

	//get a pointer to the environment that caused the fault at runtime
	struct Env* faulted_env = curenv;

	//check the faulted address, is it a table or not ?
	//If the directory entry of the faulted address is NOT PRESENT then
	if ( (curenv->env_page_directory[PDX(fault_va)] & PERM_PRESENT) != PERM_PRESENT)
	{
		// we have a table fault =============================================================
		//		cprintf("[%s] user TABLE fault va %08x\n", curenv->prog_name, fault_va);
		faulted_env->tableFaultsCounter ++ ;

		table_fault_handler(faulted_env, fault_va);
	}
	else
	{
		// we have normal page fault =============================================================
		faulted_env->pageFaultsCounter ++ ;

//				cprintf("[%08s] user PAGE fault va %08x\n", curenv->prog_name, fault_va);
//				cprintf("\nPage working set BEFORE fault handler...\n");
//				env_page_ws_print(curenv);

		if(isBufferingEnabled())
		{
			__page_fault_handler_with_buffering(faulted_env, fault_va);
		}
		else
		{
			page_fault_handler(faulted_env, fault_va);
		}
//				cprintf("\nPage working set AFTER fault handler...\n");
//				env_page_ws_print(curenv);

	}

	/*************************************************************/
	//Refresh the TLB cache
	tlbflush();
	/*************************************************************/

}


//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
	if(USE_KHEAP)
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
	else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}

}

//Handle the page fault
void page_fault_handler(struct Env * curenv, uint32 fault_va)
{
	__page_fault_handler_with_buffering(curenv, fault_va);
}
void PageFaultHandlerPlacement(struct Env * curenv, uint32 fault_va)
{
	//int IllegalAccess=0;
	uint32 *p_table;
	/*///Buffered///*/
	if(pt_get_page_permissions(curenv,fault_va)&PERM_BUFFERED)
	{
		struct Frame_Info *frame=get_frame_info(curenv->env_page_directory,(void*)fault_va,&p_table);
		pt_set_page_permissions(curenv, fault_va, PERM_PRESENT, PERM_BUFFERED);
		frame->isBuffered=0;
		if(pt_get_page_permissions(curenv, fault_va)&PERM_MODIFIED)
			bufferlist_remove_page(&modified_frame_list,frame);
		else
			bufferlist_remove_page(&free_frame_list,frame);
	}
	/*///NotBufferd///*/
	else
	{
		struct Frame_Info *frame;
		int FrameAllocatingFlag=allocate_frame(&frame);
		//if(FrameAllocatingFlag!=E_NO_MEM)
		{
			int FrameMapingFlag=map_frame(curenv->env_page_directory , frame , (void*)fault_va,PERM_WRITEABLE|PERM_USER|PERM_PRESENT);
			//if(FrameMapingFlag!=E_NO_MEM)
			{
				int read=pf_read_env_page(curenv ,(void*)fault_va);
				if(read==E_PAGE_NOT_EXIST_IN_PF)
				{
					//uint32 x=USER_HEAP_MAX,y=USTACKTOP;
					// check if it is a stack page"how??"
					//cprintf("%u %u %u\n",x,fault_va,y);
					if(fault_va>=USER_HEAP_MAX&&fault_va<USTACKTOP)
					{
						// add new empty page
						pf_add_empty_env_page(curenv,fault_va,1);
						//pf_update_env_page(curenv,(void*) fault_va, frame);
					}
					else
					{
						panic("illegal memory address");
						//IllegalAccess=1;
					}
				}
			}
		}


	}
	int NumberOfMaximumPages = curenv->page_WS_max_size;
	int index = curenv->page_last_WS_index;
	env_page_ws_set_entry(curenv,index,fault_va);
	if(index+1 ==NumberOfMaximumPages)
			curenv->page_last_WS_index = 0;
		else
					curenv->page_last_WS_index = index+1;

}


void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	//cprintf("page_fault: %u\n",fault_va);
	fault_va = ROUNDDOWN(fault_va,PAGE_SIZE);
	//cprintf("page_fault: %u\n",fault_va);
	if(env_page_ws_get_size(curenv) < curenv->page_WS_max_size)
	{
		PageFaultHandlerPlacement(curenv, fault_va);
	}
	else
	{
		//cprintf("hi\n");
		uint32 victim;
		int flag = 0;
		if  (isPageReplacmentAlgorithmModifiedCLOCK())
		{
			int CurrentIndex=curenv->page_last_WS_index;;
			int NumberOfMaximumPages = curenv->page_WS_max_size;
		while(1)
		{
			int Size = env_page_ws_get_size(curenv);
			CurrentIndex=curenv->page_last_WS_index;
			while(Size>0)
			{
				uint32 virtual=env_page_ws_get_virtual_address(curenv,CurrentIndex);
				if(!(pt_get_page_permissions(curenv,virtual)&PERM_USED) && !(pt_get_page_permissions(curenv,virtual)&PERM_MODIFIED))
				{
					flag = 1;
					victim = virtual;
					//env_page_ws_clear_entry(curenv, CurrentIndex);
					(curenv->page_last_WS_index)=CurrentIndex;
					break;

				}
				++CurrentIndex;
				CurrentIndex%=NumberOfMaximumPages;
				Size--;
			}
			if(flag)break;

			Size = env_page_ws_get_size(curenv);
			NumberOfMaximumPages = curenv->page_WS_max_size;
			CurrentIndex=curenv->page_last_WS_index;
			while(Size>0)
			{

				uint32 virtual=env_page_ws_get_virtual_address(curenv,CurrentIndex);
				if(!(pt_get_page_permissions(curenv,virtual)&PERM_USED))
				{
					flag = 1;
					victim = virtual;
					//env_page_ws_clear_entry(curenv, CurrentIndex);

					(curenv->page_last_WS_index)=CurrentIndex;

					break;

				}
				++(CurrentIndex);
				(CurrentIndex)%=NumberOfMaximumPages;
				pt_set_page_permissions(curenv,virtual,0,PERM_USED);
				Size--;
			}
			if(flag)break;
		}
		//curenv->page_last_WS_index=(CurrentIndex)%NumberOfMaximumPages;
		}
		//cprintf("%d %d\n",victim , curenv->page_last_WS_index);
		uint32 *p_table;
		struct Frame_Info *frame=get_frame_info(curenv->env_page_directory,(void*)victim,&p_table);
		frame->isBuffered=1;
		/////set the victim vitual address inside it
		frame->va = victim;
		/////set the environment inside it
		frame->environment = curenv;
		pt_set_page_permissions(curenv,victim,PERM_BUFFERED,PERM_PRESENT);

		//pt_set_page_permissions(curenv , victim,PERM_BUFFERED,PERM_PRESENT);
		if(!(pt_get_page_permissions(curenv,victim)&PERM_MODIFIED))
		{
			bufferList_add_page(&free_frame_list,frame);
			//cprintf("hi1\n");
		}
		else
		{
			bufferList_add_page(&modified_frame_list,frame);
			uint32 sizes=LIST_SIZE(& modified_frame_list);
			uint32 max_size=getModifiedBufferLength();
				//cprintf("hi2.0\n");
			if(sizes==max_size)
			{
				//cprintf("hi2.1\n");
				struct Frame_Info *ptr_fi ;
				LIST_FOREACH(ptr_fi, &modified_frame_list)
				{	//ptr_environment
					pf_update_env_page(ptr_fi->environment ,(void*)ptr_fi->va,ptr_fi);

					pt_set_page_permissions(ptr_fi->environment,ptr_fi->va,0,PERM_MODIFIED);

					bufferlist_remove_page(&modified_frame_list,ptr_fi);
					bufferList_add_page(&free_frame_list,ptr_fi);
				}
				//LIST_INIT(&modified_frame_list);
			}
		}
		//cprintf("%d ",victim)
		//env_page_ws_set_entry(curenv,curenv->page_last_WS_index , fault_va);
		//env_page_ws_invalidate(curenv, victim);
		//cprintf("page_fault: %u\n",fault_va);
		PageFaultHandlerPlacement(curenv,fault_va);
	}

	//TODO: [PROJECT 2019 - MS1 - [3] Page Fault Handler: PLACEMENT & REPLACEMENT CASES]
	// Write your code here, remove the panic and write your code
	//panic("page_fault_handler_with_buffering() is not implemented yet...!!");

	//refer to the project documentation for the detailed steps of the page fault handler

	//TODO: [PROJECT 2019 - BONUS6] Change WS Size according to “Program Priority”
}