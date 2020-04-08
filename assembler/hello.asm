# calculate primes below 100000
# runtime: 4,966s
.data
newline: .asciiz "\n"

.text
main:
    addi  $t0, $zero, 1
    li  $t1, 100000

loop:
    addi  $t2, $zero, 2

    innerLoop:
        bge     $t2, $t0, isPrime

        rem     $t3, $t0, $t2
        beqz      $t3, loopEnd

        addi     $t2, $t2, 1
        j     innerLoop

isPrime:
    addi $v0, $zero, 1
    add $a0, $t0, $zero
    syscall

    addi $v0, $zero, 4
    la $a0, newline
    syscall

loopEnd:
    addi     $t0, $t0, 1
    blt      $t0, $t1, loop

end:
#    prints      $doneStr
    addi $v0, $zero, 10
    syscall

#$doneStr       byte[]      "done", '!', 0xA

# C++ equivalent
# runtime: 1,800s (G++ 7.3.0 -O3)

# #include <stdio.h>
# #include <stdint.h>
# int main() {
#     for (uint32_t num=1# num<100000# num++) {
#         bool isPrime = true#
#         for (uint32_t i=2# i<num# i++) {
#             if (num % i == 0) {
#                 isPrime = false#
#                 break#
#             }
#         }
#         if (isPrime)
#             printf("%d\n", num)#
#     }
#     return 0#
# }


# JS (node v8.10.0)
# 2,175s (wow)

# for (num=1# num<100000# num++)
# {
#   isPrime = true#
#   for (i=2# i<num# i++)
#   {
#     if (num % i == 0)
#     {
#       isPrime = false#
#       break#
#     }
#   }

#   if (isPrime)
#     console.log(`${num}`)#
# }


# Lua 5.3
# runtime: 7,234s

# for num=1,99999 do
#   isPrime = true
#   for i=2,num-1 do
#     if num % i == 0 then
#       isPrime = false
#       break
#     end
#   end
#   if isPrime then
#     print(num)
#   end
# end


# Python 3.6.6
# runtime: 35,952s

# for num in range(1, 100001):
#     is_prime = True
#     for i in range(2, num):
#         if num % i == 0:
#             is_prime = False
#             break
#     if is_prime:
#         print(num)