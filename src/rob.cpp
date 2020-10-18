#include <stdio.h>
#include <assert.h>

#include "rob.h"


int32_t NUM_ROB_ENTRIES;

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
    return !(t->ROB_Entries[t->tail_ptr].valid);
    // if the tail doesn't have something in it, return true
}

/////////////////////////////////////////////////////////////
// insert entry at tail, increment tail (do check_space first)
/////////////////////////////////////////////////////////////

int ROB_insert(ROB *t, Inst_Info inst){
    int PRF_id = -1;
    if (ROB_check_space(t)) {
        t->ROB_Entries[t->tail_ptr].inst = inst;            // insert at the tail
        t->ROB_Entries[t->tail_ptr].valid = true;           // used spot -> mark valid
        PRF_id = t->tail_ptr;                               // return index
        t->tail_ptr = (t->tail_ptr + 1) % NUM_ROB_ENTRIES;  // wrap around
    }
    return PRF_id;  // -1 if no space
}

/////////////////////////////////////////////////////////////
// When an inst gets scheduled for execution, mark exec
/////////////////////////////////////////////////////////////

void ROB_mark_exec(ROB *t, Inst_Info inst){
    int PRF_id = inst.dr_tag;
    t->ROB_Entries[PRF_id].exec = true;
}


/////////////////////////////////////////////////////////////
// Once an instruction finishes execution, mark rob entry as done
/////////////////////////////////////////////////////////////

void ROB_mark_ready(ROB *t, Inst_Info inst){
    int PRF_id = inst.dr_tag;
    t->ROB_Entries[PRF_id].ready = true;    // ready to wrtie back and commit ?
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
    if(t->ROB_Entries[tag].ready && t->ROB_Entries[tag].valid) {
        t->ROB_Entries[tag].inst.src1_ready = true;
        t->ROB_Entries[tag].inst.src2_ready = true;
    }
}


/////////////////////////////////////////////////////////////
// Remove oldest entry from ROB (after ROB_check_head)
/////////////////////////////////////////////////////////////

Inst_Info ROB_remove_head(ROB *t){
    Inst_Info removed;
    if(ROB_check_head(t)) {
        removed = t->ROB_Entries[t->head_ptr].inst;
        t->ROB_Entries[t->head_ptr].valid = false;  // remove
        t->ROB_Entries[t->head_ptr].ready = false;
        t->head_ptr = (t->head_ptr + 1) % NUM_ROB_ENTRIES;  // wrap around
    }
    return removed;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
