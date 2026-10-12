.include "hdr.asm"

.section ".rodata1" superfree

snesfont:
.incbin "gfx/pvsneslibfont.pic"
snespal:
.incbin "gfx/pvsneslibfont.pal"

.ends

.section ".rodata2" superfree

level1pic:
.incbin "gfx/level1.pic"
level1pic_end:

level1map:
.incbin "gfx/level1.map"
level1map_end:

level1pal:
.incbin "gfx/level1.pal"
level1pal_end:

.ends

.section ".rosprite" superfree

hero0pic:
.incbin "gfx/hero0.pic"
hero0pic_end:
hero0pal:
.incbin "gfx/hero0.pal"
hero0pal_end:

hero1pic:
.incbin "gfx/hero1.pic"
hero1pic_end:
hero1pal:
.incbin "gfx/hero1.pal"
hero1pal_end:

hero2pic:
.incbin "gfx/hero2.pic"
hero2pic_end:
hero2pal:
.incbin "gfx/hero2.pal"
hero2pal_end:

hero3pic:
.incbin "gfx/hero3.pic"
hero3pic_end:
hero3pal:
.incbin "gfx/hero3.pal"
hero3pal_end:

ratpic:
.incbin "gfx/rat.pic"
ratpic_end:
ratpal:
.incbin "gfx/rat.pal"
ratpal_end:

.ends
