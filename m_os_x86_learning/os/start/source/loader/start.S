  	// 16位代码，务必加上
  	.code16
 	.text
	.global _start, loader_entry
_start:
	jmp loader_entry
