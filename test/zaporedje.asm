; to verjetno ne dela

pp start / 64
jmp start

.org 64
start:
  lbz 0
  lab

zanka:
  xci 3
  xae
  adis 1
  jmp s2
  xae
  lae
  jmp zanka

s2:
  lbz 1
  lab

z2:
  xci 3
  xae
  adis 1
  jmp start
  xae
  lae
  jmp z2
