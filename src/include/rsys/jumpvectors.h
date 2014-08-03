#if !defined(__RSYS_JUMPVECTORS__)
#define __RSYS_JUMPVECTORS__
namespace Executor {
#if !defined (JFLUSH_H)
extern HIDDEN_ProcPtr JFLUSH_H, JResUnknown1_H, JResUnknown2_H;
#endif

#define JFLUSH		(JFLUSH_H.p)
#define JResUnknown1	(JResUnknown1_H.p)
#define JResUnknown2	(JResUnknown2_H.p)
}
#endif /* !defined(__RSYS_JUMPVECTORS__) */
