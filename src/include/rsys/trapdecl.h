#if !defined (_TRAPDECL_H_)
#define _TRAPDECL_H_

#define A0(v, t0, n0)							\
v t0 n0(void)

#define A1(v, t0, n0, t1, n1)						\
v t0 n0(t1 n1)

#define A2(v, t0, n0, t1, n1, t2, n2)					\
v t0 n0(t1 n1, t2 n2)

#define A3(v, t0, n0, t1, n1, t2, n2, t3, n3)				\
v t0 n0(t1 n1, t2 n2, t3 n3)

#define A4(v, t0, n0, t1, n1, t2, n2, t3, n3, t4, n4)			\
v t0 n0(t1 n1, t2 n2, t3 n3, t4 n4)

#define A5(v, t0, n0, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5)		\
v t0 n0(t1 n1, t2 n2, t3 n3, t4 n4, t5 n5)

#define A6(v, t0, n0, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6)	\
v t0 n0(t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6)

#define A7(v, t0, n0, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7)\
v t0 n0(t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7)

#define A8(v, t0, n0, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8)\
v t0 n0(t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8)

#define A9(v, t0, n0, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9)\
v t0 n0(t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9)

#define P0(v, t0, n0)							\
v t0 C_ ## n0(void)

#define P1(v, t0, n0, t1, n1)						\
v t0 C_ ## n0(t1 n1)

#define P2(v, t0, n0, t1, n1, t2, n2)					\
v t0 C_ ## n0(t1 n1, t2 n2)

#define P3(v, t0, n0, t1, n1, t2, n2, t3, n3)				\
v t0 C_ ## n0(t1 n1, t2 n2, t3 n3)

#define P4(v, t0, n0, t1, n1, t2, n2, t3, n3, t4, n4)			\
v t0 C_ ## n0(t1 n1, t2 n2, t3 n3, t4 n4)

#define P5(v, t0, n0, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5)		\
v t0 C_ ## n0(t1 n1, t2 n2, t3 n3, t4 n4, t5 n5)

#define P6(v, t0, n0, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6)	\
v t0 C_ ## n0(t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6)

#define P7(v, t0, n0, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7)\
v t0 C_ ## n0(t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7)

#define P8(v, t0, n0, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8)\
v t0 C_ ## n0(t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8)

#define P9(v, t0, n0, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9)\
v t0 C_ ## n0(t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9)

#define P10(v, t0, n0, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10)\
v t0 C_ ## n0(t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9, t10 n10)

#define P11(v, t0, n0, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7, t8, n8, t9, n9, t10, n10, t11, n11)\
v t0 C_ ## n0(t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6, t7 n7, t8 n8, t9 n9, t10 n10, t11 n11)

#define P_SAVED0D1A0A1_0	P0
#define P_SAVED0D1A0A1_1	P1
#define P_SAVED0D1A0A1_2	P2
#define P_SAVED0D1A0A1_3	P3
#define P_SAVED0D1A0A1_4	P4
#define P_SAVED0D1A0A1_5	P5
#define P_SAVED0D1A0A1_6	P6
#define P_SAVED0D1A0A1_7	P7
#define P_SAVED0D1A0A1_8	P8
#define P_SAVED0D1A0A1_9	P9

#define P_SAVED1A0A1_0		P0
#define P_SAVED1A0A1_1		P1
#define P_SAVED1A0A1_2		P2
#define P_SAVED1A0A1_3		P3
#define P_SAVED1A0A1_4		P4
#define P_SAVED1A0A1_5		P5
#define P_SAVED1A0A1_6		P6
#define P_SAVED1A0A1_7		P7
#define P_SAVED1A0A1_8		P8
#define P_SAVED1A0A1_9		P9

#endif /* _TRAPDECL_H_ */
