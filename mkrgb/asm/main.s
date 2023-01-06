
.include "sys/gb_arch.s"

.rombankmap
;  bankstotal 64
  bankstotal 32
  banksize $4000
;  banks 64
  banks 32
.endro

.emptyfill $FF

.background "mkrgb.gb"

.computegbchecksum

;=======================================================================
; free space
;=======================================================================

; expanded ROM space (minus inaccessible bank 0x20)
.unbackground $40000 $7FFFF
;.unbackground $84000 $FFFFF

; unused routines
.unbackground $1785-$5 $17A9
.unbackground $17C6 $1809
; doPrintIdle
.unbackground $17B2 $17C5
; writeTopLineCharPart
.unbackground $1586 $15B0
; end of flee routine
.unbackground $239F $23E7

; free space at end of bank 1
.unbackground $7D60 $7FFF

;=======================================================================
; script
;=======================================================================

.include "gen/script.inc"
.include "gen/script_credits.inc"
.include "gen/script_fixedlen.inc"
.include "gen/script_misc.inc"
.include "gen/script_places.inc"

;=======================================================================
; old memory
;=======================================================================

;.define currentBank $FF00+$AE
.define currentBankB $AE

.define currentScriptOffsetLoB $B2
.define currentScriptOffsetHiB $B3

.define textSpeed $D1C2

.define currentPrintTileIndex $D266
.define targetBaseTileX $D267
.define targetBaseTileY $D268
.define savedPrintTileIndex $D27E

;=======================================================================
; old routines
;=======================================================================

.define memSet $04ED
.define memcpy $0505
.define sendToVram $0517

.define handleNextScriptChar $14CD

.define printStd $1750
.define handleNextStdChar $1756

.define setTilemapByte $180A

.define fetchScriptByte $1E54
.define switchBank $1E8E

;=======================================================================
; defines
;=======================================================================

.define extraBank $10

.define op_jump $F9
.define op_call $F8
.define op_nothing $F7
.define op_spaceTo $F6
.define op_halfBr $F5

.define op_terminator $7F
.define op_br $7D

.define op_space5px $49
.define op_tilebr $4A
.define op_space4px $4B
.define op_space1px $4E
.define op_space8px $4F

.define op_space $00

.define op_oldSpace1 $4D

.define reservedSpaceTile $8D



.define digitBase $50
.define bigDigitBase $60

.define noBankMarker $00

.define fontCharH 8
.define fontCharByteSize 8
.define charCompBufSize 8

;=======================================================================
; new mem
;=======================================================================

.define newMemStart $DE80
.define newMemEnd $DF00
.define newMemSize newMemEnd-newMemStart

.enum newMemStart
  scratchByte db
  scratchWord .dw
    scratchWordLo db
    scratchWordHi db
  scratchWord2 .dw
    scratchWord2Lo db
    scratchWord2Hi db
  
  overrideScriptBank db
  overrideScriptPtr .dw
    overrideScriptPtrLo db
    overrideScriptPtrHi db
  old_overrideScriptBank db
  old_overrideScriptPtr .dw
    old_overrideScriptPtrLo db
    old_overrideScriptPtrHi db
  
  ; current printing y/x tile coordinates
  currentPrintTileYX .dw
    currentPrintTileX db
    currentPrintTileY db
  
  ; whatever coordinates we were lasted handed by the game.
  ; x is incremented with every character, so this quickly
  ; loses parity with the actual target position;
  ; it's used to figure out when the game has moved to
  ; a new printing position (if y does not match this y,
  ; or x does not equal previous x plus one).
  lastNominalPrintTileYX .dw
    lastNominalPrintTileX db
    lastNominalPrintTileY db
  
  lastPrintTileIndex db
  
;  currentPrintChar db
  currentPrintCharW db
  
  currentPrintStartTileX db
  
  immediatePrintNeeded db
  
  printResetClearAreaStart .db
    ; 0-7, indicating pixel offset within current print tile
    currentPrintSubpixelX db
    
    compBufferPending db
    
    ; 1bpp, 8 lines of 8 pixels each
    charCompBuf ds 8
  printResetClearAreaEnd .db
  
  ; 1bpp, 8 lines of 16 pixels each
  charShiftBuf ds 16
  
  ; 2bpp, 8 lines of 8 pixels each
  outputTileBuf ds 16
  
  extraTextBuf ds 32
  
  creditsLineNum db
.ende

;=======================================================================
; macros
;=======================================================================

.macro doStdTrampolineCall ARGS addr
  call trampolineCallExtraBank
    .dw addr
.endm

;=======================================================================
; DEBUG: replace status screen with debug menu
;=======================================================================

/*.bank 0 slot 0
.orga $389D
.section "debug menu 1" overwrite
  jp z,$3D49
.ends*/

;=======================================================================
; DEBUG: new game goes to credits
;=======================================================================

/*.bank 1 slot 1
.orga $4D74
.section "credits debug 1" overwrite
  jp $683E
.ends*/

;=======================================================================
; core routines
;=======================================================================

;=================================
; bank 0 trampoline
;=================================

/*.bank 0 slot 0
.orga $014E
.section "header checksum 1" overwrite
  
.ends*/

.bank 0 slot 0
.section "trampoline 1" free
  trampolineCallExtraBank:
    ; save needed registers
    ld (scratchByte),a
    ld a,l
    ld (scratchWordLo),a
    ld a,h
    ld (scratchWordHi),a
    ld a,e
    ld (scratchWord2Lo),a
    ld a,d
    ld (scratchWord2Hi),a
    
    ; read target routine addr from after call
    pop hl
    ldi a,(hl)
    ld e,a
    ldi a,(hl)
    ld d,a
    
    ; new ret addr is original+2
    push hl
    
    ; page in target bank
    ld a,($FF00+currentBankB)
    push af
      ld a,extraBank
      call switchBank
      
      ; set up ret addr
      ld hl,trampolineCallExtraBank_end
      push hl
      
      ; set up call addr
      push de
      
      ; restore registers
      ld a,(scratchWordLo)
      ld l,a
      ld a,(scratchWordHi)
      ld h,a
      ld a,(scratchWord2Lo)
      ld e,a
      ld a,(scratchWord2Hi)
      ld d,a
      ld a,(scratchByte)
      
      ; make call
      ret
.ends

.bank 0 slot 0
.section "trampoline 2" free
  ; HL = dst
  trampolineCallExtraBank_end:
      ; save A
      ld (scratchByte),a
    ; restore bank
    pop af
    call switchBank
    ; restore A
    ld a,(scratchByte)
    ret
.ends

;=======================================================================
; printStd
;=======================================================================

;=================================
; redirect main printing
;=================================

.bank 0 slot 0
.orga $1750
.section "printStd redirect 1" SIZE 6 overwrite
  ; save current bank
  ld a,($FF00+currentBankB)
  push af
  
  ; jump to extra logic
  jp redirect_printStd
.ends

.bank 0 slot 0
.section "printStd redirect 2" free
  redirect_printStd:
    ; do normal printing
    call printStd_loop
    
    ; if comp buffer pending for end of string, send it
    call sendCompBufIfPending
    
    ; ensure original bank remains loaded,
    ; even after a bank jump
    pop af
    jp switchBank
;    ret
.ends

.bank 0 slot 0
.section "printStd redirect 3" free
  printStd_loop:
    -:
      ldi a,(hl)
      call handleNextStdChar
;      doStdTrampolineCall testPrintStd
      jr -
.ends

.bank 0 slot 0
.section "printStd redirect 4" free
  sendCompBufIfPending:
    ld a,(compBufferPending)
    or a
    jr z,+
;      ld a,op_tilebr
;      doStdTrampolineCall printChar
      push bc
      push de
      push hl
        doStdTrampolineCall sendCompBuf
        xor a
        ld (compBufferPending),a
      pop hl
      pop de
      pop bc
    +:
    ret
.ends

;=================================
; send comp buffer on linebreak
;=================================

.bank 0 slot 0
.orga $17AC
.section "printStd linebreak 1" overwrite
  jp doPrintStdLinebreak
.ends

.bank 0 slot 0
.section "printStd linebreak 2" free
  doPrintStdLinebreak:
    ; send comp buffer for end of line if needed
    call sendCompBufIfPending
    
    ; make up work
    ld e,$00
    inc d
    inc d
    ret
.ends

;=================================
; extra printStd ops
;=================================

.bank 0 slot 0
.orga $176D
.section "printStd new ops 1" SIZE 24-5 overwrite
  ; check for new ops
  cp op_nothing
  ret z
  cp op_call
  jp z,doPrintStd_callOp
;  cp op_jump
;  jp z,doPrintStd_jumpOp
  jp doPrintStd_checkMoreOps
  printStd_resume:
  doStdTrampolineCall doPrintStd_literal
  ret
  
.ends

.bank 0 slot 0
.section "printStd new ops 2" free
/*    ; load target bank
    ld a,($FF00+currentBankB)
    push af
      ldi a,(hl)
      call switchBank
      
      ; get addr
      ldi a,(hl)
      ld (scratchWordLo),a
      
      ld a,($FF00+currentBankB)
      or a
      ldi a,(hl)
      jr nz,+
      ; if current bank == 0
        ; convert to bank 0 pointer
        and $3F
      +:
      ld (scratchWordHi),a
      
      ; save ret addr for src
      push hl
        ld a,(scratchWordLo)
        ld l,a
        ld a,(scratchWordHi)
        ld h,a
        
        ; recursive call
        call printStd
      pop hl
    pop af
    jp switchBank*/
  
/*  stdPrint_setUpJump:
    ; get bank
    ldi a,(hl)
    pha
      ; get addr
      ldi a,(hl)
      ld (scratchWordLo),a
    pla
    pha
      or a
      ldi a,(hl)
      jr nz,+
      ; if current bank == 0
        ; convert to bank 0 pointer
        and $3F
      +:
      ld h,a
      ld a,(scratchWordLo)
      ld l,a
    pla
    ret*/
  
  ; HL = src
  stdPrint_setUpJump:
  doPrintStd_jumpOp:
    push bc
      ; get addr
      ld c,(hl)
      inc hl
      ld b,(hl)
      inc hl
      ; get bank
      ld a,(hl)
      
      ld l,c
      ld h,b
    pop bc
    
    ; check bank
    or a
    jr nz,+
    ; if bank == 0
      push af
        ; convert to bank 0 pointer
        ld a,$3F
        and h
        ld h,a
      pop af
    +:
;    ret
    jp switchBank
.ends

.bank 0 slot 0
.section "printStd new ops 3" free
  doPrintStd_callOp:
    ; save bank
    ld a,($FF00+currentBankB)
    push af
      ; save current pointer
      push hl
        ; read args
        call stdPrint_setUpJump
        ; set up bank using returned value
;        call switchBank
        ; recursive call
        call printStd
      ; restore old pointer
      pop hl
      ; pointer += 3 to skip op args
      inc hl
      inc hl
      inc hl
    ; restore bank
    pop af
    jp switchBank
.ends

/*.bank 0 slot 0
.section "printStd new ops 4" free
  doPrintStd_jumpOp:
    ; read args
    call stdPrint_setUpJump
    ; set up bank using returned value
    jp switchBank
.ends*/

.bank extraBank slot 1
.section "printStd new ops 5" free
  doPrintStd_literal:
    ; TODO
;    call $1586
;    call $1E94
;    call $180A
    
    call printChar
;    call doPrintIdle
    
    ; ++x
    inc e
    
    ; auto-wrap check
/*    ld a,($D269)
    dec a
    cp e
    ret nc
    ; auto-wrap
    ld e,$00
    inc d
    inc d*/
    ret 
.ends

.bank 0 slot 0
.section "printStd new ops 6" free
  doPrintStd_checkMoreOps:
    cp op_jump
    jp z,doPrintStd_jumpOp
    cp op_spaceTo
    jp z,doPrintStd_spaceToOp
    cp op_halfBr
    jp z,doPrintStd_halfBrOp
    
    jp printStd_resume
.ends

.bank 0 slot 0
.section "printStd new ops 7" free
  doPrintStd_spaceToOp:
    ldi a,(hl)
    doStdTrampolineCall doPrintStd_spaceToOp_ext
    ret
.ends

.bank extraBank slot 1
.section "printStd new ops 8" free
  doPrintStd_spaceToOp_ext:
    push bc
    push hl
      ld b,a
      
      ; B = target tile
      ld a,(currentPrintStartTileX)
      add b
      ld b,a
      
      ld a,(currentPrintTileX)
      cp b
      jr nc,@done
      -:
        ld a,op_tilebr
        call printChar
        
        ; ++nominalX
        inc e
        
        ; check if current tile x == target
        ld a,(currentPrintTileX)
        cp b
        jr c,-
    @done:
    pop hl
    pop bc
    ret
.ends

.bank 0 slot 0
.section "printStd new ops 9" free
  doPrintStd_halfBrOp:
    ; send comp buffer for end of line if needed
    call sendCompBufIfPending
    
    ld e,$00
    inc d
    ret
.ends

;=======================================================================
; printRaw
;=======================================================================

.bank 0 slot 0
.orga $1735
.section "printRaw new 1" SIZE $1B overwrite
  printRaw:
/*    -:
      ld a,(hl)
      cp $7F
      ret z
      call copyLiteralToVram [$1E94]
      call setTilemapByte [$180A]
      call doPrintIdle [$17B1]
      inc hl
      inc e
      ld a,(autoWrapWidth [$D269])
      dec a
      cp e
      jr nc,-
    ; auto-wrap
    ld e,$00
    inc d
    jr -*/
    -:
      ldi a,(hl)
      cp $7F
      jp z,sendCompBufIfPending
      cp op_br
      jr nz,+
        call sendCompBufIfPending
        ld e,$00
        inc d
        jr -
      +:
      
      doStdTrampolineCall printChar
      inc e
      jr -
    
.ends

;=======================================================================
; dialogue script handling
;=======================================================================

;=================================
; executeRegularScript redirect
;=================================

.bank 0 slot 0
.orga $14AE
.section "executeRegularScript new 1" SIZE $E overwrite
  executeRegularScript:
    ; NOTE: this call cannot happen while bank 1 is paged out --
    ; routines at $4000+ are called
    call $1827
    
    doStdTrampolineCall initRegularScript
    call executeRegularScript_loop
    jp finishRegularScript
    
.ends

.bank 0 slot 0
.section "executeRegularScript new 2" free
  executeRegularScript_loop:
    -:
      call handleNextScriptChar
      jr -
.ends

.bank 0 slot 0
.section "executeRegularScript new 3" free
  finishRegularScript:
    doStdTrampolineCall finishRegularScript_ext
    ret
.ends

.bank extraBank slot 1
.section "executeRegularScript new 4" free
  initRegularScript:
    ; initialize override bank to "none" sentinel, indicating no override
    ld a,noBankMarker
    ld (overrideScriptBank),a
    
    @finish:
    ; make up work
    ld hl,$D26B
    ldi a,(hl)
    ld e,a
    ld d,(hl)
    ret
  
  ; A = target bank
  initArbitraryScript:
    push af
      ; save old override script params
      ld a,(overrideScriptBank)
      ld (old_overrideScriptBank),a
      ld a,(overrideScriptPtrLo)
      ld (overrideScriptPtrLo),a
      ld a,(overrideScriptPtrHi)
      ld (overrideScriptPtrHi),a
    pop af
    
    ; set up new override script params
    ld (overrideScriptBank),a
    ld a,l
    ld (overrideScriptPtrLo),a
    ld a,h
    ld (overrideScriptPtrHi),a
    
    ; make up work
;    push hl
;      call initRegularScript@finish
;    pop hl
;    ret
    jr initRegularScript@finish
  
  finishRegularScript_ext:
    ; set regular params to final override params
    ; (so final jump back to original content works as expected)
    
    ; do nothing if no override occurred
    ld a,(overrideScriptBank)
    cp noBankMarker
    jr z,@done
      push de
        ; game expects absolute offset from 0x30000 (bank 0xC).
        ; compute this from final override params.
        ; could do this more efficiently (low byte doesn't even
        ; need to be computed since base is 0x00, etc.), but why bother?
        sub $0C
;        .rept 6
;          sla a
;        .endr
        .rept 2
          rrca
        .endr
        ld h,a
        ld l,$00
        
        ld a,(overrideScriptPtrLo)
        ld e,a
        ld a,(overrideScriptPtrHi)
        and $3F
        ld d,a
        
        add hl,de
        ld a,l
        ld ($FF00+currentScriptOffsetLoB),a
        ld a,h
        ld ($FF00+currentScriptOffsetHiB),a
      pop de
      
      ; mark as no override
      ld a,noBankMarker
      ld (overrideScriptBank),a
    @done:
    ret
  
  finishArbitraryScript_ext:
    ; restore old override script params
    ld a,(old_overrideScriptBank)
    ld (overrideScriptBank),a
    ld a,(old_overrideScriptPtrLo)
    ld (overrideScriptPtrLo),a
    ld a,(old_overrideScriptPtrHi)
    ld (overrideScriptPtrHi),a
    ret
.ends

;=================================
; executeArbitraryScript redirect
;=================================

.bank 0 slot 0
.orga $14BC
.section "executeArbitraryScript redirect 1" SIZE $11 overwrite
/*  ; set up override script params
  ld a,($FF00+currentBankB)
  ld (overrideScriptBank),a
  ld a,l
  ld (overrideScriptPtrLo),a
  ld a,h
  ld (overrideScriptPtrHi),a
  
  ; FIXME: jump into appropriate position of new executeRegularScript logic
  ; (after overrideScriptBank initialized to "none")
  jp $14AE*/
  
  push hl
    ; NOTE: this call cannot happen while bank 1 is paged out --
    ; routines at $4000+ are called
    call $1827
  pop hl
  ld a,($FF00+currentBankB)
  doStdTrampolineCall initArbitraryScript
;  pop hl
;  jr executeRegularScript@loop
  jp finishArbitraryScript
  
.ends

.bank 0 slot 0
.section "executeArbitraryScript redirect 2" free
  finishArbitraryScript:
    call executeRegularScript_loop
    doStdTrampolineCall finishArbitraryScript_ext
    ret
.ends

;=================================
; doPrintIdle
;=================================

.bank 0 slot 0
.orga $17B1
.section "doPrintIdle new 1" overwrite
  ; this routine never does anything; turn it into free space
  doPrintIdle:
    ret
.ends

;=================================
; fetchScriptByte
;=================================

.bank 0 slot 0
.orga $1E7A
.section "fetchActiveScriptByte new 1" SIZE $14 overwrite
  fetchActiveScriptByte:
    doStdTrampolineCall fetchActiveScriptByte_ext
    ret
.ends

.bank extraBank slot 1
.section "fetchActiveScriptByte new 2" free
  fetchActiveScriptByte_ext:
    push hl
      ; check if override active
      ld a,(overrideScriptBank)
      cp noBankMarker
      jr nz,@override
        ; prep params
        ld a,($FF00+currentScriptOffsetLoB)
        ld l,a
        ld a,($FF00+currentScriptOffsetHiB)
        ld h,a
        
        ; fetch byte
        call fetchScriptByte
        
        push af
          ; update srcptr
          ld a,l
          ld ($FF00+currentScriptOffsetLoB),a
          ld a,h
          ld ($FF00+currentScriptOffsetHiB),a
        pop af
        
        jr @done
      @override:
        push bc
          ; prep params
          ld a,(overrideScriptPtrLo)
          ld l,a
          ld a,(overrideScriptPtrHi)
          ld h,a
          ld a,(overrideScriptBank)
          ld b,a
          
          ; fetch byte
          call fetchBankByte
          
          ; update srcptr
          push af
            ld a,l
            ld (overrideScriptPtrLo),a
            ld a,h
            ld (overrideScriptPtrHi),a
          pop af
        pop bc
    @done:
    pop hl
    ret
.ends

.bank 0 slot 0
.section "fetchActiveScriptByte new 3" free
  ; B = bank
  ; HL = ptr (incremented)
  ; returns A = fetched byte
  fetchBankByte:
    ld a,($FF00+currentBankB)
    push af
      ld a,b
      call switchBank
      ldi a,(hl)
      ld b,a
    pop af
    call switchBank
    ld a,b
    ret
.ends

;=================================
; handleScriptChar
;=================================

.bank 0 slot 0
.orga $14FA
.section "handleScriptChar new 1" SIZE $B overwrite
  ; NOTE: we're overwriting the useless-to-us automatic character wrapping feature
  
  ; A = next script byte
  ; check for ops
  cp op_jump
  jp nz,$1505
    doStdTrampolineCall handleScriptJump
    ret
.ends

.bank extraBank slot 1
.section "handleScriptChar new 2" free
  handleScriptJump:
    call fetchActiveScriptByte_ext
    push af
      call fetchActiveScriptByte_ext
      push af
        call fetchActiveScriptByte_ext
        ; convert bank of 0 to FF and set pointer to bank 0
        or a
        jr nz,+
          pop af
          and $3F
          push af
          ld a,$FF
        +:
        ld (overrideScriptBank),a
      pop af
      ld (overrideScriptPtrHi),a
    pop af
    ld (overrideScriptPtrLo),a
    ret
.ends

;=======================================================================
; printing redirects
;=======================================================================

;=================================
; handleScriptChar
;=================================

.bank 0 slot 0
.orga $1505
.section "handleScriptChar printing 1" overwrite
  doStdTrampolineCall printCharDialogue
  jp $150E
.ends

;=======================================================================
; printing
;=======================================================================

.bank extraBank slot 1
.section "new printing 1" free
  ; A = char index
  ; DE = y/x
  printCharDialogue:
    push af
      ld a,e
      cp $FF
      jr nz,+
        ; the game deliberately sets x to 0xFF after e.g. a box wait
        ; in order to force an auto-wrap;
        ; we need to detect and account for this condition.
        ; do immediate linebreak
        call $1513
      +:
      
      ld a,$FF
      ld (immediatePrintNeeded),a
    pop af
    jr printChar@start
    
  ; A = char index
  ; DE = y/x
  printChar:
    push af
      xor a
      ld (immediatePrintNeeded),a
    pop af
    
    @start:
    push bc
    push de
    push hl
    
      ;=====
      ; save needed params
      ;=====
      
;      ld (currentPrintChar),a
      
      ; ignore padding character from original game
      ; (no longer needed due to changes in handling)
      cp a,op_oldSpace1
      jp z,@done
    
      ;=====
      ; check if any parameter has changed that indicates
      ; a change in printing location, necessitating a reset
      ;=====
      
      push af
        ; current tile index unchanged?
        ld a,(currentPrintTileIndex)
        ld b,a
        ld a,(lastPrintTileIndex)
        cp b
        jr nz,@doReset
        
        ; newNominalY == oldNominalY?
        ld a,(lastNominalPrintTileY)
        cp d
        jr nz,@doResetWithTileInc
        
        ; newNominalX == (oldNominalX + 1)?
        ld a,(lastNominalPrintTileX)
        inc a
        cp e
        jr nz,@doResetWithTileInc
        
        ; update nominal x
        ld (lastNominalPrintTileX),a
        
        jr @resetDone
        @doResetWithTileInc:
          call incrementTargetTileIndex
        @doReset:
          call resetForNewPrintStart
        @resetDone:
      pop af
    
      ;=====
      ; if next char is a tile break, and current subpixel is zero,
      ; send the reserved space character to the tilemap
      ; instead of using tile space for it
      ;=====
      
      cp op_space8px
      jr z,@doTileBrSpaceCheck
      cp op_tilebr
      jr nz,@noTileBrSpace
      @doTileBrSpaceCheck:
        push af
          ld a,(currentPrintSubpixelX)
          or a
          jr nz,@tileBrSpaceDone
            pop af
            
            ld a,(currentPrintTileX)
            ld e,a
            ; move to next print tile x
            inc a
            ld (currentPrintTileX),a
            
            ld a,(currentPrintTileY)
            ld d,a
            ld a,reservedSpaceTile
            call setTilemapByte
            
            jp @done
        @tileBrSpaceDone:
        pop af
      @noTileBrSpace:
    
      ;=====
      ; read in target char tile data and width,
      ; also doing bit shifts for target subpixel
      ;=====
      
      call readyNextCharPrint
      
      ; TODO: do nothing if next char width == 0?
    
      ;=====
      ; OR left buffer with comp buffer
      ;=====
      
      ld b,charCompBufSize
      ld de,charShiftBuf
      ld hl,charCompBuf
      @bufCombineLoop:
        ld a,(de)
        or (hl)
        ldi (hl),a
        
        inc de
        inc de
        dec b
        jr nz,@bufCombineLoop
      
      ;=====
      ; send comp buffer
      ;=====
      
      ld a,$FF
      ld (compBufferPending),a
      
      ld a,(immediatePrintNeeded)
      or a
      jr nz,@sendFirstBuffer
      
      ; only send if end of pattern reached
      ld a,(currentPrintSubpixelX)
      ld b,a
      ld a,(currentPrintCharW)
      add a,b
      cp 8
      jr c,@firstBufferSendDone
      
      @sendFirstBuffer:
        call sendCompBuf
        xor a
        ld (compBufferPending),a
      @firstBufferSendDone:
    
      ;=====
      ; update fields and check if right-side transfer needed
      ;=====
      
      ld a,(currentPrintSubpixelX)
      ld b,a
      ld a,(currentPrintCharW)
      add a,b
      
      ; if subpixel + w < 8, done
      cp 8
      jr nc,@rightTransferNeeded
        ld (currentPrintSubpixelX),a
        jr @done
      @rightTransferNeeded:
      
      push af
      
        ;=====
        ; move to new target tile
        ;=====
        
        ; TODO: if printing goes exactly to an 8-pixel boundary, simply incrementing
        ; here often results in a wasted pattern when the game switches to a new
        ; string (incrementing the tile index at the same time).
        ; may not matter in the end, though.
        call incrementTargetTileIndex
        call incrementTargetTilePos
      
        ;=====
        ; copy right shift buf to comp buf
        ;=====
        
        ld b,charCompBufSize
        ld de,charShiftBuf+1
        ld hl,charCompBuf
        @rightBufCopyLoop:
          ld a,(de)
          ldi (hl),a
          
          inc de
          inc de
          dec b
          jr nz,@rightBufCopyLoop
      
      ;=====
      ; update subpixel x
      ;=====
      
      pop af
      sub 8
      ld (currentPrintSubpixelX),a
      or a
      jr z,@done
      
      ;=====
      ; if resultant subpixel nonzero, send new comp buf
      ;=====
      
      ld a,$FF
      ld (compBufferPending),a
      
      ld a,(immediatePrintNeeded)
      or a
      jr z,@secondBufferSendDone
        call sendCompBuf
        xor a
        ld (compBufferPending),a
      @secondBufferSendDone:
    
    @done:
    pop hl
    pop de
    pop bc
    ret
  
  ; DE = new y/x offset
  resetForNewPrintStart:
    ld a,(currentPrintTileIndex)
    ld (lastPrintTileIndex),a
    
    ld a,e
/*    cp $FF
    jr nz,+
      ; the game deliberately sets x to 0xFF after e.g. a box wait
      ; in order to force an auto-wrap;
      ; we need to detect and account for this condition.
      ; do immediate linebreak
      call $1513
    +:*/
    ld (lastNominalPrintTileX),a
    ld (currentPrintTileX),a
    ld (currentPrintStartTileX),a
    
    ld a,d
    ld (lastNominalPrintTileY),a
    ld (currentPrintTileY),a
    
    ; clear reset area
    ld hl,printResetClearAreaStart
    ld b,printResetClearAreaEnd-printResetClearAreaStart
    xor a
    -:
      ldi (hl),a
      dec b
      jr nz,-
    
    ret
  
  clearCharCompBuf:
    ld hl,clearCharCompBuf
    ld b,charCompBufSize
    xor a
    -:
      ldi (hl),a
      dec b
      jr nz,-
    ret
  
  ; A = char index
  readyNextCharPrint:
    ; get width
    ld hl,fontWidthTable
    ld c,a
    ld b,$00
    add hl,bc
    ld a,(hl)
    ld (currentPrintCharW),a
    
    ld a,c
    cp a,op_tilebr
    jr nz,+
      ; use whatever width will advance us to the next tile
      ld a,8
      ld hl,currentPrintSubpixelX
      sub (hl)
      ld (currentPrintCharW),a
    +:
    
    ; get font data pointer
    ; index *= 8
    .rept 3
      sla c
      rl b
    .endr
    ld hl,fontData
    add hl,bc
    
;    push de
      ; copy char data
      ld b,fontCharByteSize
      ld d,h
      ld e,l
      ld hl,charShiftBuf
      ld a,(currentPrintSubpixelX)
      or a
      jr nz,@doCharShift
        ; copy raw
        -:
          ld a,(de)
          ldi (hl),a
          inc de
          xor a
          ldi (hl),a
          
          dec b
          jr nz,-
        jr @charLoadDone
      @doCharShift:
      
      ; shift right by needed number of pixels
      @lineCopyLoop:
        ld a,(currentPrintSubpixelX)
        ld c,a
        
        ld a,(de)
        inc de
        ld (hl),a
        
        xor a
        @charShiftLoop:
          srl (hl)
          rra
          dec c
          jr nz,@charShiftLoop
        
        inc hl
        ldi (hl),a
        
        dec b
        jr nz,@lineCopyLoop
    @charLoadDone:
;    pop de
    
    ret
  
  sendCompBuf:
    ;=====
    ; convert 1bpp data to output tile format
    ;=====
  
    ld hl,outputTileBuf
    ld de,charCompBuf
    ld b,charCompBufSize
    -:
      ld a,(de)
      inc de
      ldi (hl),a
      ldi (hl),a
      
      dec b
      jr nz,-
    
    ;=====
    ; write bitmap data to target vram tile
    ;=====
    
    ld a,(currentPrintTileIndex)
    push af
      ; compute vram dst
      ; multiply target index by 16
      ld c,a
      ld b,$00
      .rept 4
        sla c
        rl b
      .endr
      ; add vram base
      ld hl,$8000
      add hl,bc
      ; result to DE
      ld e,l
      ld d,h
      
      ; copy
      ld hl,outputTileBuf
      ld bc,bytesPerTile
      call sendToVram
      
    ;=====
    ; write tilemap data to target position
    ;=====
      
      ld a,(currentPrintTileX)
      ld e,a
      ld a,(currentPrintTileY)
      ld d,a
    pop af
    jp setTilemapByte
  
  incrementTargetTileIndex:
    ; move to next print index
    ld a,(currentPrintTileIndex)
    inc a
    cp $A0
    jr c,+
    ; TODO: can we do this?
;    cp $D0
    cp $E8
    jr c,++
    +:
      ld a,$A0
    ++:
    ld (currentPrintTileIndex),a
    ld (lastPrintTileIndex),a
    
    ret
  
  incrementTargetTilePos:
    ; target x += 1
    ld a,(currentPrintTileX)
    inc a
    ld (currentPrintTileX),a
    
    ret
  
  fontData:
    .incbin "out/font/font.bin"
  
  fontWidthTable:
    .incbin "out/font/fontwidth.bin"
.ends

;=======================================================================
; text adjustments
;=======================================================================

;=================================
; intro positioning
;=================================

.bank 1 slot 1
.orga $64E6
.section "intro 1" overwrite
  ; y/x
;  ld de,$0A01
  ld de,$0A00
  ; h/w
;  ld hl,$0612
  ld hl,$0814
.ends

;=================================
; title copyright message
;=================================

.bank 1 slot 1
.orga $5E18
.section "title copyright 1" overwrite
  ld hl,loc_0x6498
  ld de,$0000
  call doTitleCopyrightMsg
.ends

.bank 1 slot 1
.section "title copyright 2" free
  doTitleCopyrightMsg:
    ; why can your game boy assembler not understand that MBC1 bank 0
    ; can't go in slot 1, this is pretty fucking important
    or a
    jr nz,+
      ld hl,(loc_0x6498&$3FFF)
    +:
    
    jp doTitleCopyrightMsg_ext
.ends

.bank 0 slot 0
.section "title copyright 3" free
  doTitleCopyrightMsg_ext:
    ld a,($FF00+currentBankB)
    push af
      ld a,:loc_0x6498
      call switchBank
      call printRaw
    pop af
    jp switchBank
.ends

.bank 1 slot 1
.orga $5E0F
.section "title copyright 4" overwrite
  ; y/x
;  ld de,$0E05
  ld de,$0E03
  ; h/w
  ld hl,$030B
.ends

;=================================
; interface labels
;=================================

.bank 0 slot 0
.section "interface labels 1" free
  ; "LV"
  ref_loc_0x3B7C:
    .db op_jump
    .dw loc_0x3B7C
    .db :loc_0x3B7C
.ends

.bank 0 slot 0
.section "interface labels 2" free
  ; "HP"
  ref_loc_0x3B7F:
    .db op_jump
    .dw loc_0x3B7F
    .db :loc_0x3B7F
.ends

.bank 0 slot 0
.section "interface labels 3" free
  ; "MP"
  ref_loc_0x3B82:
    .db op_jump
    .dw loc_0x3B82
    .db :loc_0x3B82
.ends

;=================================
; interface label usage
;=================================

.bank 0 slot 0
.orga $24EE
.section "interface label use 1" overwrite
  ld hl,ref_loc_0x3B7F
.ends

.bank 0 slot 0
.orga $2506
.section "interface label use 2" overwrite
  ld hl,ref_loc_0x3B82
.ends

.bank 0 slot 0
.orga $3A78
.section "interface label use 3" overwrite
  ld hl,ref_loc_0x3B7C
.ends

.bank 0 slot 0
.orga $3A8A
.section "interface label use 4" overwrite
  ld hl,ref_loc_0x3B7F
.ends

.bank 0 slot 0
.orga $3AA2
.section "interface label use 5" overwrite
  ld hl,ref_loc_0x3B82
.ends

.bank 1 slot 1
.orga $42E6
.section "interface label use 6" overwrite
  ld hl,ref_loc_0x3B7F
.ends

.bank 1 slot 1
.orga $42FE
.section "interface label use 7" overwrite
  ld hl,ref_loc_0x3B82
.ends

;=======================================================================
; increase default text speed
;=======================================================================

.bank 0 slot 0
.orga $16F9
.section "default text speed 1" overwrite
  ; ??? is this even used?
;  ld b,$08
  ld b,$04
.ends

.bank 0 slot 0
.orga $16FD
.section "default text speed 2" overwrite
  call halveTextSpeed
.ends

.bank 0 slot 0
.section "default text speed 3" free
  halveTextSpeed:
    ; make up work
    ld a,(textSpeed)
    ; halve
    srl a
    ret
.ends

;=======================================================================
; digit in save file confirmation message
;=======================================================================

.bank 1 slot 1
.orga $40CE
.section "save message 1" overwrite
  call doSaveFileConfirmMsg
  jp $40E7
.ends

.bank 1 slot 1
.section "save message 2" free
  doSaveFileConfirmMsg:
    ; save target file index
    push af
      ; print first part of message
      ld hl,$40F1
      ld de,$0100
      call printStd
    pop af
    
    ; convert target index to 1-based digit
    add a,digitBase+1
    ld hl,$D2CD
    ldi (hl),a
    ; add terminator
    ld a,op_terminator
    ldd (hl),a
    
    ; print
    call printStd
    
    ; print new end message
    ld hl,loc_0x40F2
    ld a,:loc_0x40F2
    or a
    jr nz,+
      ld hl,(loc_0x40F2&$3FFF)
    +:
    jp printStdBanked
.ends

.bank 0 slot 0
.section "save message 3" free
  ; NOTE: trashes B
  printStdBanked:
    ld b,a
    ld a,($FF00+currentBankB)
    push af
      ld a,b
      call switchBank
      call printStd
    pop af
    jp switchBank
.ends

;=======================================================================
; flee messages
;=======================================================================

.bank 0 slot 0
.orga $239C
.section "flee message 1" overwrite
  jp doFleeMessage
.ends

.bank 1 slot 1
.section "flee message 2" free
  doFleeMessage:
    ; copy in needed message
    ld a,($C401)
    bit 0,a
    jr nz,@cantFlee
      push de
      ld a,$0A
      call $4031
      and a
      pop de
      jr z,@cantFlee
        ; successful escape
        
        ; copy message
        ld hl,loc_0x20C1
        ld a,:loc_0x20C1
        or a
        jr nz,+
          ld hl,(loc_0x20C1&$3FFF)
        +:
        ld bc,32
        ld de,extraTextBuf
        call memcpy_banked1
        
        ; copy monster name
        ld de,extraTextBuf+10
        call monsterNameCopy
        
        ; show message
        ld a,$0D
        call $2F13
        ld hl,extraTextBuf
        call $23E8
        call $2CA8
        ret 
    @cantFlee:
    ; copy message
    ld hl,loc_0x20CB
    ld a,:loc_0x20CB
    or a
    jr nz,+
      ld hl,(loc_0x20CB&$3FFF)
    +:
    ld bc,32
    ld de,extraTextBuf
    call memcpy_banked1
    
    ; copy monster name
    ld de,extraTextBuf+13
    call monsterNameCopy
    
    ; show message
    ld a,$02
    call $2F13
    ld hl,extraTextBuf
    call $23E8
    jp $201F
  
  monsterNameCopy:
    ld hl,$C46E
    ld bc,$0008
    jp memcpy
.ends

.bank 0 slot 0
.section "flee message 3" free
  memcpy_banked1:
    call switchBank
    call memcpy
    ld a,$01
    jp switchBank
.ends

;=======================================================================
; "forest" area name
; (can't be done in-place because original string is too short)
;=======================================================================

.bank 1 slot 1
.orga $4419
.section "forest area name 1" overwrite
  .dw newForestAreaNameRef
.ends

.bank 1 slot 1
.section "forest area name 2" free
  newForestAreaNameRef:
    ; length
    .db 5
    .db op_call
    .dw loc_0x4479
    .db :loc_0x4479
    .db $7F
.ends

;=======================================================================
; title menu positioning
;=======================================================================

.bank 1 slot 1
.orga $5DFD
.section "title menu position 1" overwrite
  ; menu text x/y
;  ld de,$0808
  ld de,$0807
;  ld hl,$0405
  ld hl,$0405
.ends

.bank 1 slot 1
.orga $5E3F
.section "title menu position 2" overwrite
  ; cursor x/y
;  ld de,$0808
  ld de,$0807
;  ld hl,$0405
  ld hl,$0405
.ends

;=======================================================================
; digit conversion 1
;=======================================================================

.bank 0 slot 0
.orga $1FF1
.section "digit conversion 1" overwrite
  ; space index
  ld d,op_space8px
  ; digit base index
  ld e,bigDigitBase
.ends

;=======================================================================
; sound test
;=======================================================================

;=================================
; music title positioning
;=================================

.bank 1 slot 1
.orga $6149
.section "sound test music title 1" overwrite
  ; y/x
;  ld de,$0603
  ld de,$0600
  ; h/w
;  ld hl,$0212
  ld hl,$0212
.ends

;=================================
; voice title positioning
;=================================

.bank 1 slot 1
.orga $61A1
.section "sound test voice title 1" overwrite
  ; y/x
;  ld de,$0B03
  ld de,$0B00
  ; h/w
;  ld hl,$0212
  ld hl,$0212
.ends

;=================================
; vram positioning
;=================================

/*.bank 1 slot 1
.orga $5F5D
.section "sound test music vram pos 1" overwrite
  ; "sound mode"
;  ld a,$C4
  ld a,$C4
.ends*/

.bank 1 slot 1
.orga $6199
.section "sound test music vram pos 4" overwrite
  ; voice num
;  ld a,$B1
  ld a,$B8
.ends

.bank 1 slot 1
.orga $61C9
.section "sound test music vram pos 5" overwrite
  ; voice title
;  ld a,$B3
  ld a,$D0
.ends

;=======================================================================
; title screen
;=======================================================================

;=================================
; load
;=================================

.slot 1
.section "title 1" superfree
  loadTitleGrp:
    ld bc,titleGrp_size
    ld de,$C400
    ld hl,titleGrp
    call memcpy
    
    ; make up work
    ld hl,$C400
    ld de,$9000
    ld bc,$0800
    jp sendToVram
    
  loadTitleMap:
    ld bc,titleMap_size
    ld de,$C400
    ld hl,titleMap
    call memcpy
    
    ; make up work
    ld hl,$C400
    ld de,$9801
    ret
    
  titleGrp:
    .incbin "out/grp/title_bg.bin" FSIZE titleGrp_size
    
  titleMap:
    .incbin "out/maps/title.bin" FSIZE titleMap_size
.ends

.bank 1 slot 1
.orga $5DC1
.section "title 2" overwrite
  doStdTrampolineCall loadTitleGrp
  jp $5DCD
.ends

.bank 1 slot 1
.orga $5DD9
.section "title 3" overwrite
  doStdTrampolineCall loadTitleMap
  nop
.ends

;=================================
; extend shine animation to match new logo
;=================================

.bank 1 slot 1
.orga $65D7
.section "title shine 1" overwrite
  ; initial line
  ld h,$0C
  ; final line
  ld l,$3C+$10
.ends

;=======================================================================
; credits
;=======================================================================

;=================================
; title message positioning
;=================================

.bank 1 slot 1
.orga $6DBD
.section "credits title msg pos 1" overwrite
  ; y/x
  ld de,$FD06
;  ld de,$FFFD
.ends

;=================================
; correct generation of top line for title message
;=================================

.bank 1 slot 1
.orga $6DA4
.section "credits title msg top line 1" overwrite
  jp doCreditsTitleTopLine
.ends

.bank 1 slot 1
.section "credits title msg top line 2" free
  doCreditsTitleTopLine:
    push de
      ; empty line at top of buffer $D1E6
      ld hl,$D1E6
      ld bc,$0009
;      ld a,$4D
      ld a,op_space8px
      call memSet
    pop de
    
    ; add linebreak
    ld a,op_halfBr
    ldi (hl),a
    
    ; copy params
;    ld b,$09
    ld b,$08
    -:
      ld a,(de)
      inc de
      ldi (hl),a
      dec b
      jr nz,-
    
    ; skip last character (in case buffer isn't large enough for it
    ; now that we've added a linebreak)
    inc de
    
    ; add terminator
    ld a,$7F
    ld (hl),a
    
    jp $6DBC
.ends

;=================================
; title message top line blanking
;=================================

/*.bank 1 slot 1
.orga $6DD4
.section "credits title msg blanking 1" overwrite
  call doCreditsTitleTopLineBlank
.ends

.bank 1 slot 1
.section "credits title msg blanking 2" free
  doCreditsTitleTopLineBlank:
    push de
      dec d
      ld hl,creditsBlankLine
      call printStd
    pop de
    
    ; make up work
    inc d
    ld a,$03
    ret
  
  creditsBlankLine:
    .rept 9
      .db op_space8px
    .endr
    .db op_terminator
.ends*/

;=================================
; content msg pos
;=================================

.bank 1 slot 1
.orga $6DE7
.section "credits content msg pos 1" overwrite
  ; y/x
;  ld de,$0414
  ld de,$0314
  ; h/w
  ld hl,$0202
.ends

;=================================
; content msg start
;=================================

.bank 1 slot 1
.orga $6DF1
.section "credits content msg start 1" overwrite
  jp setUpNextContentMsg
;  jp $6DFD
.ends

.bank 1 slot 1
.section "credits content msg start 2" free
  setUpNextContentMsg:
    ; save start of string content;
    ; it will be used to advance to the next op
    ; when done
    push de
    
    ; copy to new src buffer
    
    ; skip first param byte (assumed to be jump or call redirect)
    inc de
    
    ; hl = redirect target ptr
    ld a,(de)
    ld l,a
    inc de
    ld a,(de)
    ld h,a
    ; a = bank
    inc de
    ld a,(de)
    push af
      ; if zero, modify pointer to target slot 0
      or a
      jr nz,+
        ld a,$3F
        and h
        ld h,a
      +:
      
      ; bc = size
      ld bc,$0020
      ; de = dst
      ld de,extraTextBuf
    pop af
    call memcpy_banked1
    
    ; use text that was copied to extraTextBuf as src
    ld de,extraTextBuf
    
    xor a
    ld (creditsLineNum),a
    
    ; make up work
    ; char 1
;    ld a,$4D
    ld a,op_space8px
    ld ($D1E7),a
    ; char 2
;    ld a,$7F
    ld a,op_terminator
    ld ($D1E8),a
    ; originally number of src chars to iterate over;
    ; now simply used to figure out character's target position
    ld b,$10
;    ld b,$20
    jp $6DFD
.ends

;=================================
; content msg end
;=================================

.bank 1 slot 1
.orga $6E51
.section "credits content msg end 1" overwrite
  jp endNextContentMsg
.ends

.bank 1 slot 1
.section "credits content msg end 2" free
  endNextContentMsg:
    ; pop start of string content
    pop de
    ; add 16 to advance past params
    ld a,$10
    add e
    ld e,a
    jr nc,+
      inc d
    +:
    
    ; make up work
    pop af
    ld (currentPrintTileIndex),a
    jp $6E55
.ends

;=================================
; content msg processing
;=================================

.bank 1 slot 1
.orga $6DFD
.section "credits content msg processing 1" overwrite
  creditsContentMsgLoopStart:
    jp doCreditsContentMsgLoopStart
.ends

.bank 1 slot 1
.section "credits content msg processing 2" free
  doCreditsContentMsgLoopStart:
    ; make up work
    ; read next char
    ld a,(de)
    inc de
    
    ; check for ops
    ; linebreak
    cp op_br
    jr nz,+
      ; increment line num
      ld a,(creditsLineNum)
      inc a
      ld (creditsLineNum),a
      
      ; reset target line pos
      ld b,$11
      jp $6E4E
    +:
    
    ; terminator
    cp op_terminator
    jr nz,+
      jp $6E51
    +:
    
    ; make up work
    ; character to treat as space and ignore
  ;  cp $4D
    cp op_space8px
    jp $6E01
;    jp z,$6E4E
;    jp $6E03
.ends

.bank 1 slot 1
.orga $6E08
.section "credits content msg processing 3" overwrite
  call doCreditsContentMsgPrintStart
.ends

.bank 1 slot 1
.section "credits content msg processing 4" free
  doCreditsContentMsgPrintStart:
    ; make up work
    ; y/x offset
    ld de,$0100
    
    ; offset y by current line num
    ld a,(creditsLineNum)
    add d
    ld d,a
    ret
.ends

.bank 1 slot 1
.orga $6E11
.section "credits content msg processing 5" overwrite
  call doCreditsContentMsgPrintAfter
.ends

.bank 1 slot 1
.section "credits content msg processing 6" free
  doCreditsContentMsgPrintAfter:
    ; make up work
    ; y/x offset for target
    ld de,$0503
    
    ; offset y by current line num
    ld a,(creditsLineNum)
    add d
    ld d,a
    ret
.ends

.bank 1 slot 1
.orga $6E4F
.section "credits content msg processing 7" overwrite
  ; loop unconditionally instead of using B as counter
  jr creditsContentMsgLoopStart
.ends

.bank 1 slot 1
.orga $6E42
.section "credits content msg processing 8" overwrite
  ; frame delay between each time character is moved
  ld a,$01
.ends

;=================================
; final copyright msg
;=================================

.bank 1 slot 1
.orga $6EB9
.section "credits final msg 1" overwrite
  ; blank char
;  ld a,$4D
  ld a,op_space8px
.ends

.bank 1 slot 1
.orga $6EBE
.section "credits final msg 2" overwrite
  jp doCreditsFinalMsgSetup
.ends

.bank 1 slot 1
.section "credits final msg 3" free
  doCreditsFinalMsgSetup:
    ; add half br
    ; make up work
    pop de
    
    ; add linebreak
    ld a,op_halfBr
    ldi (hl),a
    
;    ld b,$18
    ld b,$17
    -:
      ld a,(de)
      inc de
      ldi (hl),a
      dec b
      jr nz,-
    
    ; skip last character
    inc de
    
    jp $6EC7
.ends

;=======================================================================
; adjust intro timing to maintain music sync
;=======================================================================

; the new text takes longer to print than the original,
; so the timing has to be slightly tweaked so the main part
; of the title theme still kicks in right as the title comes up

;=================================
; time per screen
;=================================

.bank 1 slot 1
.orga $64FF
.section "intro timing 1" overwrite
  ; frame count / 2?
;  ld b,$C8
  ld b,$C4
.ends

;=======================================================================
; laboriously hand-correct SGB attribute map for title screen
;=======================================================================

.bank 1 slot 1
.orga $5C87
.section "title sgb attr map 1" overwrite
/*  .db $00,$00,$0A,$AA,$87
  .db $00,$00,$0A,$AA,$81
  .db $00,$00,$0A,$AA,$15
  .db $00,$00,$00,$01,$50
  .db $54,$00,$00,$01,$50
  .db $58,$00,$00,$01,$68
  .db $00,$00,$00,$02,$50
  .db $00,$00,$00,$01,$50
  .db $00,$00,$00,$00,$00
  .db $00,$00,$00,$00,$00
  .db $00,$00,$00,$00,$00
  .db $00,$00,$00,$00,$00
  .db $00,$00,$00,$00,$00
  .db $40,$00,$00,$00,$00
  .db $40,$00,$00,$00,$00
  .db $40,$00,$00,$00,$00
  .db $40,$00,$00,$00,$00
  .db $D4,$00,$00,$00,$00*/
  
/*  .db $00,$AA,$AA,$AA,$87
  .db $00,$AA,$AA,$AA,$81
  .db $00,$00,$0A,$AA,$55
  .db $00,$00,$00,$00,$50
  .db $00,$00,$00,$00,$54
  .db $00,$00,$00,$00,$50
  .db $00,$00,$00,$00,$54
  .db $00,$00,$00,$00,$50
  .db $00,$00,$00,$00,$00
  .db $00,$00,$00,$00,$00
  .db $00,$00,$00,$00,$00
  .db $00,$00,$00,$00,$00
  .db $00,$00,$00,$00,$00
  .db $40,$00,$00,$00,$00
  .db $40,$00,$00,$00,$00
  .db $40,$00,$00,$00,$00
  .db $40,$00,$00,$00,$00
  .db $D4,$00,$00,$00,$00*/
  
  .db $00,$AA,$AA,$AA,$87
  .db $00,$AA,$AA,$AA,$81
  .db $00,$AA,$AA,$AA,$55
  .db $00,$00,$00,$00,$54
  .db $00,$00,$00,$00,$54
  .db $00,$00,$00,$00,$54
  .db $00,$00,$00,$00,$54
  .db $00,$00,$00,$00,$54
  .db $00,$00,$00,$00,$50
  .db $00,$00,$00,$00,$00
  .db $00,$00,$00,$00,$00
  .db $00,$00,$00,$00,$00
  .db $00,$00,$00,$00,$00
  .db $40,$00,$00,$00,$00
  .db $40,$00,$00,$00,$00
  .db $40,$00,$00,$00,$00
  .db $40,$00,$00,$00,$00
  .db $D4,$00,$00,$00,$00
.ends

/*.bank 1 slot 1
.orga $5CE1
.section "title sgb attr map 2" overwrite
  .db $15,$55,$55,$55,$54
  .db $40,$00,$00,$00,$01
  .db $40,$00,$00,$00,$01
  .db $40,$00,$00,$00,$01
  .db $40,$00,$00,$00,$01
  .db $40,$00,$00,$00,$01
  .db $40,$00,$00,$00,$01
  .db $40,$00,$00,$00,$01
  .db $40,$00,$00,$00,$01
  .db $15,$55,$55,$55,$54
  .db $FF,$FF,$AA,$FF,$FF
  .db $FF,$FF,$AA,$FF,$FF
  .db $FF,$FF,$AA,$FF,$FF
  .db $FF,$FF,$AA,$FF,$FF
  .db $FF,$FF,$FF,$FF,$FF
  .db $FF,$FF,$FF,$FF,$FF
  .db $FF,$FF,$FF,$FF,$FF
  .db $FF,$FF,$FF,$FF,$FF
.ends*/
