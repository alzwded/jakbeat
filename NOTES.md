Objective
=========

Sample based drum sequencer.

TODO
====

* [x] read sample PCM flt 44.1khz
  + [/] read any PCM data
* [x] input deck
  + [x] parser
  + [x] sample setup
  + [x] phrase setup
  + [x] phrase beats
  + [ ] stereo kit setup
* [x] PCM flt 44.1khz output
  + [ ] live output
  + [x] stereo output
    - [ ] dynamically load plugins

Input deck
==========

Grammar
-------

```
file ::= sections ;
sections ::= | sections section ;
section ::= title options ;
title ::= '[' STRING ']' ;
options ::= | options option ;
options ::= STRING '=' value ;
value ::= STRING
        | '(' items ')'
        ;
items ::= ;
        | items item
        ;
item ::= value
       | option
       ;

STRING: "([^\s]+\|"[^"]*")"
```

WHO
---

```
[WHO]
kick = (
    path = kick.wav
    volume = 80
    pan = 0
)
snare = (
    path = "snare.wav"
    volume = 50
    pan = -10
)
crash = (
    path = ../crash.wav
    volume = 40
    pan = 60
)
ride = (
    path = "c:\my files\ride.wav"
    volume = 20
    pan = 30
)
```

WHAT
----

```
[WHAT]
A1 = ( bpm = 120 )
A2 = ( bpm = 120 )
Output = (A1 A2 A1 A1)
```

WHERE
-----

```
[A1]
kick =  !...!-.-!...!-..
snare = ..!...!...!...!.
hat =   !.-.!.-.!.-.!.-.
crash = ...............!
```
