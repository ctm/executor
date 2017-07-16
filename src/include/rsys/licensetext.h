#if !defined (_RSYS_LICENSETEXT_H_)
#define _RSYS_LICENSETEXT_H_
namespace Executor {
typedef struct
{
  const char *heading;
  const char *body;
} license_text_page_t;

extern license_text_page_t ROMlib_license[];
}
#endif /* _RSYS_LICENSETEXT_H_ */
