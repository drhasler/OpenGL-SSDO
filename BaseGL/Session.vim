let SessionLoad = 1
if &cp | set nocp | endif
let s:so_save = &so | let s:siso_save = &siso | set so=0 siso=0
let v:this_session=expand("<sfile>:p")
silent only
silent tabonly
cd ~/lix/inf584/BaseGL
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
set shortmess=aoO
argglobal
%argdel
edit Sources/ShaderProgram.h
set splitbelow splitright
wincmd _ | wincmd |
vsplit
1wincmd h
wincmd _ | wincmd |
split
1wincmd k
wincmd w
wincmd w
wincmd _ | wincmd |
split
1wincmd k
wincmd w
set nosplitbelow
set nosplitright
wincmd t
set winminheight=0
set winheight=1
set winminwidth=0
set winwidth=1
exe '1resize ' . ((&lines * 15 + 33) / 66)
exe 'vert 1resize ' . ((&columns * 140 + 106) / 213)
exe '2resize ' . ((&lines * 48 + 33) / 66)
exe 'vert 2resize ' . ((&columns * 140 + 106) / 213)
exe '3resize ' . ((&lines * 25 + 33) / 66)
exe 'vert 3resize ' . ((&columns * 72 + 106) / 213)
exe '4resize ' . ((&lines * 38 + 33) / 66)
exe 'vert 4resize ' . ((&columns * 72 + 106) / 213)
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
let s:l = 50 - ((8 * winheight(0) + 7) / 15)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
50
normal! 0
wincmd w
argglobal
if bufexists("Sources/Main.cpp") | buffer Sources/Main.cpp | else | edit Sources/Main.cpp | endif
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 61 - ((12 * winheight(0) + 24) / 48)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
61
normal! 0
wincmd w
argglobal
if bufexists("Resources/Shaders/geometry.vs") | buffer Resources/Shaders/geometry.vs | else | edit Resources/Shaders/geometry.vs | endif
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 1 - ((0 * winheight(0) + 12) / 25)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
1
normal! 0
wincmd w
argglobal
if bufexists("Resources/Shaders/geometry.fs") | buffer Resources/Shaders/geometry.fs | else | edit Resources/Shaders/geometry.fs | endif
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 1 - ((0 * winheight(0) + 19) / 38)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
1
normal! 0
wincmd w
2wincmd w
exe '1resize ' . ((&lines * 15 + 33) / 66)
exe 'vert 1resize ' . ((&columns * 140 + 106) / 213)
exe '2resize ' . ((&lines * 48 + 33) / 66)
exe 'vert 2resize ' . ((&columns * 140 + 106) / 213)
exe '3resize ' . ((&lines * 25 + 33) / 66)
exe 'vert 3resize ' . ((&columns * 72 + 106) / 213)
exe '4resize ' . ((&lines * 38 + 33) / 66)
exe 'vert 4resize ' . ((&columns * 72 + 106) / 213)
tabnext 1
badd +112 Sources/Main.cpp
badd +1 Resources/Shaders/mixer.fs
badd +2 Sources/ShaderProgram.h
badd +1 Resources/Shaders/geometry.fs
badd +0 Resources/Shaders/geometry.vs
if exists('s:wipebuf') && len(win_findbuf(s:wipebuf)) == 0
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20 shortmess=filnxtToOSA
set winminheight=1 winminwidth=1
let s:sx = expand("<sfile>:p:r")."x.vim"
if file_readable(s:sx)
  exe "source " . fnameescape(s:sx)
endif
let &so = s:so_save | let &siso = s:siso_save
nohlsearch
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
