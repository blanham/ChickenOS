.global _start
.extern main

_start:

jmp  main
.wait: hlt
jmp .wait
