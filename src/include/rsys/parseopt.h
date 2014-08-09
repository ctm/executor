#if !defined (_RSYS_PARSEOPT_H_)
#define _RSYS_PARSEOPT_H_
namespace Executor {
extern boolean_t parse_system_version (std::string vers);
	extern boolean_t parse_size_opt (std::string opt, std::string  arg);
extern boolean_t parse_prres_opt (Executor::INTEGER *outx, Executor::INTEGER *outy,
				  std::string  arg);
extern boolean_t ROMlib_parse_version (std::string vers, uint32 *version_out);


#define CREATE_SYSTEM_VERSION(a, b, c) \
  ((((a) & 0xF) << 8) | (((b) & 0xF) << 4) | ((c) & 0xF))
}
#endif /* !_RSYS_PARSEOPT_H_ */
