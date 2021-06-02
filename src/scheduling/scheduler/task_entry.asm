[bits 64]
[extern _Z9task_exitv]
[global task_entry]

; NOTE: the addres to the target task needs to be in rax
; we do this to prevent jumping to random code if the task returns

task_entry:
	call rax

	call _Z9task_exitv ; if the task ever returns just exit
	
	jmp $