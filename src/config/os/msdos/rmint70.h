#if !defined (_RMINT70_H_)
#define _RMINT70_H_

extern uint8 rm_int_70_handler_start;
extern uint8 rm_int_70_handler_code_start;
extern uint8 rm_int_70_handler_end;
extern __dpmi_raddr rm_int_70_chain_address;

extern uint16 rm_int_70_seg;
extern uint8 rm_elapsed_1024_placeholder;
extern uint8 rm_remaining_1024_placeholder;

#endif /* !_RMINT70_H_ */
