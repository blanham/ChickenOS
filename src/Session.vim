let SessionLoad = 1
let s:so_save = &so | let s:siso_save = &siso | set so=0 siso=0
let v:this_session=expand("<sfile>:p")
silent only
cd ~/projects/ChickenOS/src
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
set shortmess=aoO
badd +74 init/kmain.c
badd +30 arch/i386/thread.c
badd +163 thread/thread.c
badd +114 thread/thread_ops.c
badd +1 thread/fork.c
badd +77 include/kernel/thread.h
badd +186 arch/i386/intr-core.s
badd +73 thread/scheduler.c
badd +1 device/timer.c
badd +14 init/init.c
badd +88 mm/vm.c
badd +76 fs/vfs.c
badd +377 ~/projects/ChickenOS/log
badd +49 device/interrupt.c
badd +36 arch/i386/interrupt.c
badd +78 fs/ops.c
badd +105 arch/i386/paging.c
badd +7 lib/string.c
badd +223 thread/exec.c
badd +146 mm/regions.c
badd +34 ~/projects/ChickenOS/good.log
badd +14 thread/syscall.c
badd +21 arch/i386/switch.s
badd +17 GOALS
badd +49 ~/projects/ChickenOS/GOALS
badd +12 thread/signal.c
badd +1 mm/mm_ops.c
badd +8 arch/i386/time.c
badd +28 include/kernel/timer.h
badd +13 device/ata.c
badd +23 include/chicken/time.h
badd +1 /tmp/nvimTsavR2/3
badd +116 device/time.c
badd +5 ~/.vimrc
badd +76 arch/i386/multiboot.c
badd +204 include/multiboot.h
badd +27 ~/projects/ChickenOS/scripts/grub.cfg
badd +15 include/chicken/boot.h
badd +38 include/chicken/vbe.h
badd +57 include/kernel/common.h
badd +7 include/chicken/list.h
badd +1 1
badd +5 arch/i386/syscall.c
badd +8 include/assert.h
badd +581 include/util/utlist.h
badd +3 include/arch/i386/common.h
badd +7 ~/projects/ChickenOS/LICENSE
badd +1 /tmp/nvimTsavR2/4
badd +42 mm/frame.c
badd +48 include/mm/vm.h
badd +14 include/mm/paging.h
badd +25 arch/i386/i386_defs.h
badd +1 arch/i386/gdt.c
badd +25 device/chardevs.c
badd +4 include/arch/i386/gdt.h
badd +8 include/arch/i386/idt.h
badd +3 include/thread/tss.h
badd +8 arch/i386/tss.c
badd +1 arch/i386/intr-core.
badd +1 mm/palloc.
badd +87 mm/palloc.c
badd +0 term://.//31094:/bin/zsh
badd +47 thread/load_elf.c
argglobal
silent! argdel *
argadd init/kmain.c
edit mm/regions.c
set splitbelow splitright
wincmd _ | wincmd |
vsplit
1wincmd h
wincmd _ | wincmd |
split
wincmd _ | wincmd |
split
2wincmd k
wincmd w
wincmd w
wincmd w
wincmd _ | wincmd |
split
wincmd _ | wincmd |
split
wincmd _ | wincmd |
split
3wincmd k
wincmd w
wincmd w
wincmd w
set nosplitbelow
set nosplitright
wincmd t
set winheight=1 winwidth=1
exe '1resize ' . ((&lines * 44 + 25) / 51)
exe 'vert 1resize ' . ((&columns * 102 + 102) / 204)
exe '2resize ' . ((&lines * 1 + 25) / 51)
exe 'vert 2resize ' . ((&columns * 102 + 102) / 204)
exe '3resize ' . ((&lines * 2 + 25) / 51)
exe 'vert 3resize ' . ((&columns * 102 + 102) / 204)
exe '4resize ' . ((&lines * 11 + 25) / 51)
exe 'vert 4resize ' . ((&columns * 101 + 102) / 204)
exe '5resize ' . ((&lines * 12 + 25) / 51)
exe 'vert 5resize ' . ((&columns * 101 + 102) / 204)
exe '6resize ' . ((&lines * 12 + 25) / 51)
exe 'vert 6resize ' . ((&columns * 101 + 102) / 204)
exe '7resize ' . ((&lines * 11 + 25) / 51)
exe 'vert 7resize ' . ((&columns * 101 + 102) / 204)
argglobal
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 105 - ((19 * winheight(0) + 22) / 44)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
105
normal! 030|
wincmd w
argglobal
edit mm/vm.c
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 41 - ((0 * winheight(0) + 0) / 1)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
41
normal! 038|
wincmd w
argglobal
edit GOALS
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 77 - ((0 * winheight(0) + 1) / 2)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
77
normal! 09|
wincmd w
argglobal
edit term://.//31094:/bin/zsh
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
let s:l = 1011 - ((10 * winheight(0) + 5) / 11)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
1011
normal! 01|
wincmd w
argglobal
edit ~/projects/ChickenOS/log
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 39 - ((6 * winheight(0) + 6) / 12)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
39
normal! 019|
wincmd w
argglobal
edit init/init.c
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 12 - ((3 * winheight(0) + 6) / 12)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
12
normal! 025|
wincmd w
argglobal
edit thread/load_elf.c
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 47 - ((3 * winheight(0) + 5) / 11)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
47
normal! 031|
wincmd w
exe '1resize ' . ((&lines * 44 + 25) / 51)
exe 'vert 1resize ' . ((&columns * 102 + 102) / 204)
exe '2resize ' . ((&lines * 1 + 25) / 51)
exe 'vert 2resize ' . ((&columns * 102 + 102) / 204)
exe '3resize ' . ((&lines * 2 + 25) / 51)
exe 'vert 3resize ' . ((&columns * 102 + 102) / 204)
exe '4resize ' . ((&lines * 11 + 25) / 51)
exe 'vert 4resize ' . ((&columns * 101 + 102) / 204)
exe '5resize ' . ((&lines * 12 + 25) / 51)
exe 'vert 5resize ' . ((&columns * 101 + 102) / 204)
exe '6resize ' . ((&lines * 12 + 25) / 51)
exe 'vert 6resize ' . ((&columns * 101 + 102) / 204)
exe '7resize ' . ((&lines * 11 + 25) / 51)
exe 'vert 7resize ' . ((&columns * 101 + 102) / 204)
tabnext 1
if exists('s:wipebuf') && getbufvar(s:wipebuf, '&buftype') isnot# 'terminal'
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20 shortmess=filnxtToOc
let s:sx = expand("<sfile>:p:r")."x.vim"
if file_readable(s:sx)
  exe "source " . fnameescape(s:sx)
endif
let &so = s:so_save | let &siso = s:siso_save
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
