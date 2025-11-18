/*
   TYPES.H  by Paolo Pancheri       (281097)
   Home Page: http://www.geocities.com/SiliconValley/Park/8933
   EMail: DarkAngel@tin.it

   Standard types
   For DJGPP 2.0
*/

#ifndef __TYPES__
#define __TYPES__

#define BYTE unsigned char
#define WORD unsigned short int
#define DWORD unsigned int
#define BOOL signed char
#define TRUE 1
#define FALSE 0

#define PACKED __attribute__ ((packed))
#pragma pack(1)

#define PRIVATE 					static
#define END_OF_FUNCTION(x)    void x##_end() { }
#define LOCK_VARIABLE(x)      _go32_dpmi_lock_data((void *)&x, sizeof(x))
#define LOCK_FUNCTION(x)      _go32_dpmi_lock_code(x, (long)x##_end - (long)x)
#define INSTALL_HANDLER(x, y) _go32_dpmi_get_protected_mode_interrupt_vector(x, &y##_old); \
   									y##_new.pm_offset = (int)y; \
   									y##_new.pm_selector = _go32_my_cs(); \
   									_go32_dpmi_allocate_iret_wrapper(&y##_new); \
   									_go32_dpmi_set_protected_mode_interrupt_vector(x, &y##_new);
#define REMOVE_HANDLER(x, y)  _go32_dpmi_set_protected_mode_interrupt_vector(x, &y##_old); \
                              _go32_dpmi_free_iret_wrapper(&y##_new);



#endif
