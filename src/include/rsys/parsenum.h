#if !defined (_RSYS_PARSENUM_H_)
#define _RSYS_PARSENUM_H_

namespace Executor {
extern boolean_t parse_number (std::string orig_num, int32 *val,
							   unsigned round_up_to_multiple_of);
}
#endif /* !_RSYS_PARSENUM_H_ */
