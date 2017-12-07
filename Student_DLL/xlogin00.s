; Login studenta: xharmi00
; Dominik Harmim <xharmi00@stud.fit.vutbr.cz>

[BITS 32]

GLOBAL DllMain
EXPORT DllMain

GLOBAL ipa_algorithm
EXPORT ipa_algorithm


SECTION .data


SECTION .text

DllMain:
	ENTER 0, 0

	MOV EAX, DWORD 1

	LEAVE
	RET 12


ipa_algorithm:
	ENTER 0, 0

	%define input_data [EBP + 8]
	%define output_data [EBP + 12]
	%define width [EBP + 16]
	%define height [EBP + 20]
	%define argc [EBP + 24]
	%define argv [EBP + 28]

	; TODO

	LEAVE
	RET 0
