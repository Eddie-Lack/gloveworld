.ifndef _MACRO_INC_GUARD
.internal _MACRO_INC_GUARD
.set _MACRO_INC_GUARD, 1

.macro glabel name
    .global \name
    .type \name, @function
    \name:
.endm

.macro dlabel name
    .global \name
    .type \name, @object
    \name:
.endm

.macro jlabel name
    .global \name
    \name:
.endm

.macro alabel name
    .global \name
    \name:
.endm

.macro endlabel name
    .size \name, . - \name
.endm

.macro enddlabel name
    .size \name, . - \name
.endm

.macro nonmatching name, size=1
.endm

.endif
