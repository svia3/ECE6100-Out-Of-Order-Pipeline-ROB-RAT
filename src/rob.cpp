#include <stdio.h>
#include <assert.h>

#include "rob.h"


extern int32_t NUM_ROB_ENTRIES;

/////////////////////////////////////////////////////////////
// Init function initializes the ROB
/////////////////////////////////////////////////////////////

ROB* ROB_init(void){
    int ii;
    ROB *t = (ROB *) calloc (1, sizeof (ROB));
    for(ii=0; ii<MAX_ROB_ENTRIES; ii++){
        t->ROB_Entries[ii].valid=false;
        t->ROB_Entries[ii].ready=false;
        t->ROB_Entries[ii].exec=false;
    }
    t->head_ptr=0;
    t->tail_ptr=0;
    return t;
}

/////////////////////////////////////////////////////////////
// Print State
/////////////////////////////////////////////////////////////
void ROB_print_state(ROB *t){
 int ii = 0;
  printf("Printing ROB \n");
  printf("Entry  Inst   Valid   ready\n");
  for(ii = 0; ii < 7; ii++) {
    printf("%5d ::  %d\t", ii, (int)t->ROB_Entries[ii].inst.inst_num);
    printf(" %5d\t", t->ROB_Entries[ii].valid);
    printf(" %5d\n", t->ROB_Entries[ii].ready);
    printf(" %5d\n", t->ROB_Entries[ii].exec);
  }
  printf("\n");
}

/////////////////////////////////////////////////////////////
// If there is space in ROB return true, else false
/////////////////////////////////////////////////////////////

bool ROB_check_space(ROB *t){
    // return (t->tail_ptr != t->head_ptr) && t->ROB_Entries[head_ptr]
    // std::cout << "Space" << !(t->ROB_Entries[t->tail_ptr].valid) <<"\n";
    return !(t->tail_ptr == t->head_ptr && t->ROB_Entries[t->head_ptr].valid);
    // if the head is valid -> something in it -> no space
    // head at the tail -> full
}

/////////////////////////////////////////////////////////////
// insert entry at tail, increment tail (do check_space first)
/////////////////////////////////////////////////////////////

int ROB_insert(ROB *t, Inst_Info inst){
    int PRF_id = -1;
    if (ROB_check_space(t)) {
        // build the entry --
        t->ROB_Entries[t->tail_ptr].inst = inst;            // insert at the tail
        t->ROB_Entries[t->tail_ptr].valid = true;           // used spot -> mark valid
        t->ROB_Entries[t->tail_ptr].exec = false;           // execute?
        t->ROB_Entries[t->tail_ptr].ready = false;          // ready?
        PRF_id = t->tail_ptr;                               // return index
        t->tail_ptr = (t->tail_ptr + 1) % NUM_ROB_ENTRIES;  // wrap around
    }
    return PRF_id;  // -1 if no space
}

/////////////////////////////////////////////////////////////
// When an inst gets scheduled for execution, mark exec
/////////////////////////////////////////////////////////////

void ROB_mark_exec(ROB *t, Inst_Info inst){
    if (t->ROB_Entries[inst.dr_tag].valid) {
        t->ROB_Entries[inst.dr_tag].exec = true;
    }
}


/////////////////////////////////////////////////////////////
// Once an instruction finishes execution, mark rob entry as done
/////////////////////////////////////////////////////////////

void ROB_mark_ready(ROB *t, Inst_Info inst){
    if (t->ROB_Entries[inst.dr_tag].valid) {
        t->ROB_Entries[inst.dr_tag].ready = true;
    }
}

/////////////////////////////////////////////////////////////
// Find whether the prf (rob entry) is ready
/////////////////////////////////////////////////////////////

bool ROB_check_ready(ROB *t, int tag){
    return t->ROB_Entries[tag].ready && t->ROB_Entries[tag].valid;
}


/////////////////////////////////////////////////////////////
// Check if the oldest ROB entry is ready for commit
/////////////////////////////////////////////////////////////

bool ROB_check_head(ROB *t){
    return t->ROB_Entries[t->head_ptr].ready && t->ROB_Entries[t->head_ptr].valid;
}

/////////////////////////////////////////////////////////////
// For writeback of freshly ready tags, wakeup waiting inst
/////////////////////////////////////////////////////////////

void  ROB_wakeup(ROB *t, int tag){
    // if(t->ROB_Entries[tag].ready && t->ROB_Entries[tag].valid) {
    //     t->ROB_Entries[tag].inst.src1_ready = true;
    //     t->ROB_Entries[tag].inst.src2_ready = true;
    // }
    // -- CDB broadcast that is waiting for that tag value to be produced --
    // int curr = t.head_ptr;
    // int size = ROB_get_size(t);
    int size = ROB_get_size(t);
    for(int j = 0; j < size; j++) {
        int k = (t->head_ptr + j) % NUM_ROB_ENTRIES;
        if(tag == t->ROB_Entries[k].inst.src1_tag && t->ROB_Entries[k].valid) {
            t->ROB_Entries[k].inst.src1_ready = true;
        }
        if(tag == t->ROB_Entries[k].inst.src2_tag && t->ROB_Entries[k].valid) {
            t->ROB_Entries[k].inst.src2_ready = true;
        }
    }
}


/////////////////////////////////////////////////////////////
// Remove oldest entry from ROB (after ROB_check_head)
/////////////////////////////////////////////////////////////

Inst_Info ROB_remove_head(ROB *t) {
    Inst_Info head = t->ROB_Entries[t->head_ptr].inst;
    t->ROB_Entries[t->head_ptr].valid = false;  // remove
    t->ROB_Entries[t->head_ptr].ready = false;
    t->ROB_Entries[t->head_ptr].exec = false;
    t->head_ptr = (t->head_ptr + 1) % NUM_ROB_ENTRIES;  // wrap around
    return head;
}

/////////////////////////////////////////////////////////////
// Get size of ROB for looping iteration count
/////////////////////////////////////////////////////////////
int ROB_get_size(ROB *t) {
    if (!ROB_check_space(t)) {
        return NUM_ROB_ENTRIES;
    } else if (t->tail_ptr - t->head_ptr >= 0) {
        return t->tail_ptr - t->head_ptr;
    } else {
        return (NUM_ROB_ENTRIES - t->head_ptr) + (t->tail_ptr);
    }
}
