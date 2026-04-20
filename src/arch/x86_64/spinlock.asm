[bits 64]
section .text



global acquireLockWithFlags
acquireLockWithFlags:
	pushfq
	cli
	pop rax
	mov [rsi], rax ; we saved flags
	; abuse falltrough

global acquireLock
acquireLock:
.try_lock
	lock bts dword [rdi],0        ;Attempt to acquire the lock (in case lock is uncontended)
	jc .spin_with_pause
	ret

.spin_with_pause:
	pause			; Tell CPU we're spinning
	test dword [rdi],1	; Is the lock free?
	jnz .spin_with_pause	; no, wait
	jmp .try_lock		; retry


acquireTryLockWithFlags:
	pushfq
	cli

	lock bts dword [rdi],0        ;Attempt to acquire the lock (in case lock is uncontended)
	jnc .success

	popfq
	mov eax, 0
	jmp .ret

.success:
	pop rax
	mov [rsi], rax ; we saved flags
	mov eax, 1
.ret
	ret

acquireTryLock:
	lock bts dword [rdi],0        ;Attempt to acquire the lock (in case lock is uncontended)
	jnc .success
	mov eax, 0
	jmp .ret
.success:
	mov eax, 1
.ret
	ret
	

global releaseLock
releaseLock:
	mov dword [rdi],0
	ret

global releaseLockWithFlags
releaseLockWithFlags:
	mov dword [rdi],0

	mov rax, [rsi] ; restore the flags
	push rax
	popfq
	ret

global testLock 
testLock:
	mov eax, dword [rdi]
	and eax, 1
	ret



global acquireReadLockWithFlags
acquireReadLockWithFlags:
	pushfq
	cli
	pop rax
	mov [rsi], rax ; we saved flags
	; abuse falltrough
global acquireReadLock
acquireReadLock:
.wait_no_write
	test byte [rdi+4], 1
	jz .try_add_reader
	pause
	jmp .wait_no_write
.try_add_reader:
	lock add dword [rdi], 1
	test byte [rdi+4], 1
	jz .lock_acquired
	lock sub dword [rdi], 1
	pause
	jmp .wait_no_write
.lock_acquired
	ret


global releaseReadLock
releaseReadLock:
	lock sub dword [rdi], 1
	ret

global releaseReadLockWithFlags
releaseReadLockWithFlags:
	lock sub dword [rdi], 1
.lock_released
	mov rax, [rsi] ; restore the flags
	push rax
	popfq
	ret


global acquireWriteLockWithFlags
acquireWriteLockWithFlags:
	pushfq
	cli
	pop rax
	mov [rsi], rax ; we saved flags

global acquireWriteLock
acquireWriteLock:
.wait_no_write:
	lock bts dword [rdi+4],0
	jc .spin_wait_no_write
	jmp .wait_no_read
.spin_wait_no_write:
	pause
	test byte [rdi+4], 1
	jnz .spin_wait_no_write
	jmp .wait_no_write

.wait_no_read:
	cmp dword [rdi], 0
	je .lock_acquired
	pause
	jmp .wait_no_read
.lock_acquired
	ret

global releaseWriteLock
releaseWriteLock:
	mov byte [rdi+4],0
	ret

global releaseWriteLockWithFlags
releaseWriteLockWithFlags:
	mov byte [rdi+4],0
.lock_released
	mov rax, [rsi] ; restore the flags
	push rax
	popfq
	ret
