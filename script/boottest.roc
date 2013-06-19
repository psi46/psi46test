mf $02000020 $100000 0
flush
bootstrap
mdelay 5000
ms $02000020 $10000 xboot02000020.txt
flush
memsave $02000020 $10000 xboot02000020.bin
flush
memsave $02000020 $10000 xorig02000020.bin