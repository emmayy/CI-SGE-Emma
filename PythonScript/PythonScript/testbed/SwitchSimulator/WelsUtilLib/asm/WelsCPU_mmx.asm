;/***********************************************************************
;*	Copyright (c) 2004-2010 WebEx Communications, Inc.
;*  All rights reserved	
;*
;*	cpu_mmx.asm
;*
;*  Author
;*      Steven Huang(stevenh@hz.webex.com)
;*
;*  Abstract
;*		verify cpuid feature support and cpuid detection
;*
;*  History
;*      04/29/2009	Created
;*
;*************************************************************************/

bits 32

;******************************************************************************************
; Macros
;******************************************************************************************

%macro WELS_EXTERN 1
	%ifdef PREFIX
		global _%1
		%define %1 _%1
	%else
		global %1
	%endif
%endmacro

;******************************************************************************************
; Code
;******************************************************************************************

SECTION .text

; refer to "The IA-32 Intel(R) Architecture Software Developers Manual, Volume 2A A-M"
; section CPUID - CPU Identification

WELS_EXTERN wels_cpuid_verify
ALIGN 16
;******************************************************************************************
;   int32_t wels_cpuid_verify()
;******************************************************************************************
wels_cpuid_verify:
    pushfd					; decrease the SP by 4 and load EFLAGS register onto stack, pushfd 32 bit and pushf for 16 bit
	pushfd					; need push 2 EFLAGS, one for processing and the another one for storing purpose
    pop     ecx				; get EFLAGS to bit manipulation
    mov     eax, ecx		; store into ecx followed
    xor     eax, 00200000h	; get ID flag (bit 21) of EFLAGS to directly indicate cpuid support or not
	xor		eax, ecx		; get the ID flag bitwise, eax - 0: not support; otherwise: support
    popfd					; store back EFLAGS and keep unchanged for system
    ret

WELS_EXTERN wels_cpu_cpuid
ALIGN 16
;****************************************************************************************************
;   void wels_cpu_cpuid( int32_t index, int32_t *feature_a, int32_t *feature_b, int32_t *feature_c, int32_t *feature_d )
;****************************************************************************************************
wels_cpu_cpuid:
	push	ebx	
	push	edi
	
	mov     eax, [esp+12]	; operating index
    cpuid					; cpuid
	
	; processing various information return
	mov     edi, [esp+16]
    mov     [edi], eax
    mov     edi, [esp+20]
    mov     [edi], ebx
    mov     edi, [esp+24]
    mov     [edi], ecx
    mov     edi, [esp+28]
    mov     [edi], edx

	pop		edi	
    pop     ebx
	ret




