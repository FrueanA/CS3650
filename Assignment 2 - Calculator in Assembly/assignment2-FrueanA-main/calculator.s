# A terminal calculator
#
# Reads a line of input, interprets it as a simple arithmetic expression,
# and prints the result. The input format is
# <long_integer> <operation> <long_integer>

# Make `main` accessicle outside of this module
.global main

# Start of the code section
.text

main:
  # Function prologue
  enter $0, $0

  # Use scanf to retrieve and process a line of input
  # This clock implements the following line of C code: 
  #   scanf("%ld %c %ld", &a, &op, &b);
  # Take a look at the man page for scanf and ask questions. You can also look 
  # at scanf_example.c
  movq $scanf_fmt, %rdi
  movq $a, %rsi
  movq $op, %rdx
  movq $b, %rcx
  xorb %al, %al
  call scanf

  movb op(%rip), %cl # TODO: load the operation for comparisons
  movq a(%rip), %rax  # TODO: and the LHS
  movq b(%rip), %rbx  # RHS

  # TODO: Analyze operation and execute

  # TODO: Print result

  # TODO: Print error if operation cannot be (safely) performed

  # if (op_char == '+')
  cmpb $'+', %cl
  je add_ab

  # else if (op_char == '-')
  cmpb $'-', %cl
  je sub_ab

  # else if (op_char = '*')
  cmpb $'*', %cl
  je mul_ab

  # else if (op_char == '/')
  cmpb $'/', %cl
  je div_ab

  # else - unknown operation
  jmp unknown_op

  # Add LHS and RHS and store in rcx.
  add_ab:
  addq %rbx, %rax
  jmp print_result

  #Subtract RHS from LHS and store in rcx.
  sub_ab:
  subq %rbx, %rax
  jmp print_result

  #Multiply LHS and RHS and store in rcx.
  mul_ab:
  imulq %rbx, %rax
  jmp print_result

  #Divide LHS by RHS and store in rcx unless RHS is a zero.
  div_ab:
  cmpq $0, %rbx
  je div_zero
  cqto
  idivq %rbx
  jmp print_result

  # Returns divide by zero error and returns 1.
  div_zero:
  movq $div_zero_msg, %rdi
  movb $0, %al
  call printf
  movq $1, %rax
  jmp function_ep

  # Prints the calculated result (sets 0 on success) 
  print_result:
  movq $output_fmt, %rdi
  movq %rax, %rsi
  movb $0, %al
  call printf
  movq $0, %rax
  jmp function_ep

  # Prints unknown operation error message (sets 1 on error)
  unknown_op:
  movq $unknown_op_msg, %rdi
  movb $0, %al
  call printf
  movq $1, %rax
  jmp function_ep
  
  function_ep:
  # Function epilogue
  leave
  ret

# Start of the data section
.data

output_fmt: 
  .asciz "%ld\n"
scanf_fmt: 
  .asciz "%ld %c %ld"  # TODO: modify as needed

  # errors
  div_zero_msg:
  .asciz "Division by zero\n"

  unknown_op_msg:
  .asciz "Unknown operation\n"

# "Slots" for scanf
a:  .quad 0
b:  .quad 0
op: .byte 0

