[bits 64]
[extern _Z9task_exitv]
[global task_entry]

; NOTE: the addres to the target task needs to be in rax
; we do this to prevent jumping to random code if the task returns

;# task_entry-signature: void task_entry()
;# task_entry-doc: The main entry point for a task. Marks the bottom of the new stack and jumps to the adress in rax.

task_entry:
	sti
	mov rbp, 0 ; mark bottom of stack trace
	call rax

.exit:
	call _Z9task_exitv ; if the task ever returns just exit
	
	jmp $