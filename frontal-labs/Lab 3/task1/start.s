section .data
    newline db 10
    infile_descriptor dd 0      ; default stdin (0)
    outfile_descriptor dd 1     ; default stdout (1)

section .bss
    input_buffer resb 1

section .text
    global main
    extern strlen

prepare_input:
    pushad
    mov eax, 5          ; sys_open
    mov ebx, ecx        ; points to "-i..."
    add ebx, 2          ; skip "-i"
    xor ecx, ecx        ; O_RDONLY = 0
    int 0x80
    mov [infile_descriptor], eax
    popad
    jmp check_command

prepare_output:
    pushad
    mov eax, 5          ; sys_open
    mov ebx, ecx        ; points to "-o..."
    add ebx, 2          ; skip "-o"
    mov ecx, 1          ; O_WRONLY
    or ecx, 64          ; O_CREAT
    mov edx, 0o777      ; permissions
    int 0x80
    mov [outfile_descriptor], eax
    popad
    jmp check_command

main:
    mov esi, [esp + 4]  ; argc
    mov edi, [esp + 8]  ; argv
    xor edx, edx        ; counter = 0

print_loop:
    cmp edx, esi
    jge encoder_loop

    mov ecx, [edi + edx*4] ; argv[i]

    ; print argv[i]
    pushad
    mov ebx, 1
    mov ecx, [edi + edx*4]
    push ecx
    call strlen
    pop ecx
    mov edx, eax
    mov eax, 4
    int 0x80

    ; print newline
    mov eax, 4
    mov ebx, 1
    mov ecx, newline
    mov edx, 1
    int 0x80
    popad

    ; check for -i
    cmp word [ecx], "-i"     ; "-i"
    je prepare_input

    ; check for -o
    cmp word [ecx], "-o"     ; "-o"
    je prepare_output

check_command:
    inc edx
    jmp print_loop

encoder_loop:
    ; read 1 byte
    mov eax, 3
    mov ebx, [infile_descriptor]
    mov ecx, input_buffer
    mov edx, 1
    int 0x80

    cmp eax, 0
    jle exit_program

    ; check if input_buffer is in range 'A' to 'Z'
    movzx eax, byte [input_buffer]
    cmp eax, 'A'
    jl not_in_range
    cmp eax, 'Z'
    jg not_in_range

    ; subtract 1 from character
    dec byte [input_buffer]

not_in_range:
    ; write character
    mov eax, 4
    mov ebx, [outfile_descriptor]
    mov ecx, input_buffer
    mov edx, 1
    int 0x80
    jmp encoder_loop

exit_program:
    mov eax, 1
    xor ebx, ebx
    int 0x80



    ; section .data
;     newline db 10
;     infile_descriptor dd 0      ; default stdin (0)
;     outfile_descriptor dd 1     ; default stdout (1)

; section .bss
;     input_buffer resb 1

; section .text
;     global main
;     extern strlen

; prepare_input:
;     pushad
;     mov eax, 5          ; sys_open
;     mov ebx, ecx        ; points to "-i..."
;     add ebx, 2          ; skip "-i"
;     xor ecx, ecx        ; O_RDONLY = 0
;     int 0x80
;     mov [infile_descriptor], eax
;     popad
;     jmp check_command

; prepare_output:
;     pushad
;     mov eax, 5          ; sys_open
;     mov ebx, ecx        ; points to "-o..."
;     add ebx, 2          ; skip "-o"
;     mov ecx, 1          ; O_WRONLY
;     or ecx, 64          ; O_CREAT
;     mov edx, 0o777      ; permissions
;     int 0x80
;     mov [outfile_descriptor], eax
;     popad
;     jmp check_command

; main:
;     mov esi, [esp + 4]  ; argc
;     mov edi, [esp + 8]  ; argv
;     xor edx, edx        ; counter = 0

; print_loop:
;     cmp edx, esi
;     jge encoder_loop

;     mov ecx, [edi + edx*4] ; argv[i]

;     ; print argv[i]
;     pushad
;     mov ebx, 1
;     mov ecx, [edi + edx*4]
;     push ecx
;     call strlen
;     pop ecx
;     mov edx, eax
;     mov eax, 4
;     int 0x80

;     ; print newline
;     mov eax, 4
;     mov ebx, 1
;     mov ecx, newline
;     mov edx, 1
;     int 0x80
;     popad

;     ; check for -i
;     cmp word [ecx], 0x692D     ; "-i"
;     je prepare_input

;     ; check for -o
;     cmp word [ecx], 0x6F2D     ; "-o"
;     je prepare_output

; check_command:
;     inc edx
;     jmp print_loop

; encoder_loop:
;     ; read 1 byte
;     mov eax, 3
;     mov ebx, [infile_descriptor]
;     mov ecx, input_buffer
;     mov edx, 1
;     int 0x80

;     cmp eax, 0
;     jle exit_program

;     ; check if input_buffer is in range 'A' to 'Z'
;     movzx eax, byte [input_buffer]
;     cmp eax, 'A'
;     jl not_in_range
;     cmp eax, 'Z'
;     jg not_in_range

;     ; cyclic backward shift: 'A' -> 'Z', else -1
;     cmp al, 'A'
;     je wrap_to_Z
;     dec byte [input_buffer]
;     jmp continue_encoding

; wrap_to_Z:
;     mov byte [input_buffer], 'Z'

; continue_encoding:

; not_in_range:
;     ; write character
;     mov eax, 4
;     mov ebx, [outfile_descriptor]
;     mov ecx, input_buffer
;     mov edx, 1
;     int 0x80
;     jmp encoder_loop

; exit_program:
;     mov eax, 1
;     xor ebx, ebx
;     int 0x80

