
.code32

nop
jmp -1
jmp *%eax
jmp *(%eax)
jmp *-4(%eax)
jmp *4(%eax)
jmp *-400(%eax)
jmp *400(%eax)
jmp *11223344(%eax)
jmp *-11223344

