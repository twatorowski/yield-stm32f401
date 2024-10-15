/**
 * @file scb.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-07-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef STM32F401_SCB_H
#define STM32F401_SCB_H

#include "stm32f401.h"

/* base addreses */
#define SCB_SCS_BASE                                        0xe000e000U
#define SCB_BASE                                            0xe000ed00U

/* instances */
#define SCB_SCS                                             ((scb_scs_t *)SCB_SCS_BASE)
#define SCB                                                 ((scb_t *)SCB_BASE)

/* register bank */
typedef struct {
    reg32_t CPUID;
    reg32_t ICSR;
    reg32_t VTOR;
    reg32_t AIRCR;
    reg32_t SCR;
    reg32_t CCR;
    reg8_t SHP[12U];
    reg32_t SHCSR;
    reg32_t CFSR;
    reg32_t HFSR;
    reg32_t DFSR;
    reg32_t MMFAR;
    reg32_t BFAR;
    reg32_t AFSR;
    reg32_t PFR[2U];
    reg32_t DFR;
    reg32_t ADR;
    reg32_t MMFR[4U];
    reg32_t ISAR[5U];
    reg32_t RESERVED0[5U];
    reg32_t CPACR;
} scb_t;

/********* bit definitions for SCB CPUID Register *********/
#define SCB_CPUID_IMPLEMENTER                               (0xFFUL << 24)          
#define SCB_CPUID_VARIANT                                   (0xFUL << 20)               
#define SCB_CPUID_ARCHITECTURE                              (0xFUL << 16)          
#define SCB_CPUID_PARTNO                                    (0xFFFUL << 4)              
#define SCB_CPUID_REVISION                                  (0xFUL << 0)          

/********* bit definitions for SCB Interrupt Control State Register *********/
#define SCB_ICSR_NMIPENDSET                                 (1UL << 31)               
#define SCB_ICSR_PENDSVSET                                  (1UL << 28)                
#define SCB_ICSR_PENDSVCLR                                  (1UL << 27)                
#define SCB_ICSR_PENDSTSET                                  (1UL << 26)                
#define SCB_ICSR_PENDSTCLR                                  (1UL << 25)                
#define SCB_ICSR_ISRPREEMPT                                 (1UL << 23)               
#define SCB_ICSR_ISRPENDING                                 (1UL << 22)               
#define SCB_ICSR_VECTPENDING                                (0x1FFUL << 12)          
#define SCB_ICSR_RETTOBASE                                  (1UL << 11)                
#define SCB_ICSR_VECTACTIVE                                 (0x1FFUL << 0)       

/********* bit definitions for SCB Vector Table Offset Register *********/
#define SCB_VTOR_TBLOFF                                     (0x1FFFFFFUL << 7)           

/********* bit definitions for SCB Application Interrupt and Reset Control Register *********/
#define SCB_AIRCR_VECTKEY                                   (0xFFFFUL << 16)            
#define SCB_AIRCR_VECTKEYSTAT                               (0xFFFFUL << 16)
#define SCB_AIRCR_VECTKEY_WR                                (0x05FAUL << 16)        
#define SCB_AIRCR_VECTKEY_RD                                (0xFA05UL << 16)
#define SCB_AIRCR_ENDIANESS                                 (1UL << 15)               
#define SCB_AIRCR_PRIGROUP                                  (7UL << 8)                
#define SCB_AIRCR_SYSRESETREQ                               (1UL << 2)             
#define SCB_AIRCR_VECTCLRACTIVE                             (1UL << 1)           
#define SCB_AIRCR_VECTRESET                                 (1UL << 0)           

/********* bit definitions for SCB System Control Register *********/
#define SCB_SCR_SEVONPEND                                   (1UL << 4)                 
#define SCB_SCR_SLEEPDEEP                                   (1UL << 2)                 
#define SCB_SCR_SLEEPONEXIT                                 (1UL << 1)               

/********* bit definitions for SCB Configuration Control Register *********/
#define SCB_CCR_STKALIGN                                    (1UL << 9)                  
#define SCB_CCR_BFHFNMIGN                                   (1UL << 8)                 
#define SCB_CCR_DIV_0_TRP                                   (1UL << 4)                 
#define SCB_CCR_UNALIGN_TRP                                 (1UL << 3)               
#define SCB_CCR_USERSETMPEND                                (1UL << 1)              
#define SCB_CCR_NONBASETHRDENA                              (1UL << 0)        

/********* bit definitions for SCB System Handler Control and State Register *********/
#define SCB_SHCSR_USGFAULTENA                               (1UL << 18)             
#define SCB_SHCSR_BUSFAULTENA                               (1UL << 17)             
#define SCB_SHCSR_MEMFAULTENA                               (1UL << 16)             
#define SCB_SHCSR_SVCALLPENDED                              (1UL << 15)            
#define SCB_SHCSR_BUSFAULTPENDED                            (1UL << 14)          
#define SCB_SHCSR_MEMFAULTPENDED	                        (1UL << 13)          
#define SCB_SHCSR_USGFAULTPENDED                            (1UL << 12)          
#define SCB_SHCSR_SYSTICKACT                                (1UL << 11)              
#define SCB_SHCSR_PENDSVACT                                 (1UL << 10)               
#define SCB_SHCSR_MONITORACT                                (1UL << 8)              
#define SCB_SHCSR_SVCALLACT                                 (1UL << 7)               
#define SCB_SHCSR_USGFAULTACT                               (1UL << 3)             
#define SCB_SHCSR_BUSFAULTACT                               (1UL << 1)             
#define SCB_SHCSR_MEMFAULTACT                               (1UL << 0)         

/********* bit definitions for SCB Configurable Fault Status Register *********/
#define SCB_CFSR_USGFAULTSR                                 (0xFFFFUL << 16)          
#define SCB_CFSR_BUSFAULTSR                                 (0xFFUL << 8)            
#define SCB_CFSR_MEMFAULTSR                                 (0xFFUL <<0)        

/********* MemManage Fault Status Register (part of SCB Configurable Fault Status Register) */
#define SCB_CFSR_MMARVALID                                  (1UL << 7)                
#define SCB_CFSR_MLSPERR                                    (1UL << 5)                  
#define SCB_CFSR_MSTKERR                                    (1UL << 4)                  
#define SCB_CFSR_MUNSTKERR                                  (1UL << 3)                
#define SCB_CFSR_DACCVIOL                                   (1UL << 1)                 
#define SCB_CFSR_IACCVIOL                                   (1UL << 0)             

/********* BusFault Status Register (part of SCB Configurable Fault Status Register) */
#define SCB_CFSR_BFARVALID                                  (1UL << 15)                 
#define SCB_CFSR_LSPERR                                     (1UL << 13)                    
#define SCB_CFSR_STKERR                                     (1UL << 12)                    
#define SCB_CFSR_UNSTKERR                                   (1UL << 11)                  
#define SCB_CFSR_IMPRECISERR                                (1UL << 10)               
#define SCB_CFSR_PRECISERR                                  (1UL << 9)                 
#define SCB_CFSR_IBUSERR                                    (1UL << 8)                   

/********* UsageFault Status Register (part of SCB Configurable Fault Status Register) */
#define SCB_CFSR_DIVBYZERO                                  (1UL << 25)                 
#define SCB_CFSR_UNALIGNED                                  (1UL << 24)                 
#define SCB_CFSR_NOCP                                       (1UL << 19)                      
#define SCB_CFSR_INVPC                                      (1UL << 18)                     
#define SCB_CFSR_INVSTATE                                   (1UL << 17)                  
#define SCB_CFSR_UNDEFINSTR                                 (1UL << 16)                

/********* bit definitions for SCB Hard Fault Status Register *********/
#define SCB_HFSR_DEBUGEVT                                   (1UL << 31)                 
#define SCB_HFSR_FORCED                                     (1UL << 30)                   
#define SCB_HFSR_VECTTBL                                    (1UL << 1)                  

/********* bit definitions for SCB Debug Fault Status Register *********/
#define SCB_DFSR_EXTERNAL                                   (1UL << 4)                 
#define SCB_DFSR_VCATCH                                     (1UL << 3)                   
#define SCB_DFSR_DWTTRAP                                    (1UL << 2)                  
#define SCB_DFSR_BKPT                                       (1UL << 1)                     
#define SCB_DFSR_HALTED                                     (1UL << 0)               


/* set exception priority */
#define SCB_SETEXCPRI(e, p)                     \
    (SCB->SHP[(e) - 3] = (p))

/* get exception priority */
#define SCB_GETEXCPRI(e)                        \
    (SCB->SHP[(e) - 3])

#endif /********* STM32F401_SCB_H */
